#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Box.H>
#include <sqlite3.h>
#include <ctime>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <iostream>

using namespace std;

// Database helper class
class EncryptedDB {
    sqlite3* db;
    
public:
    EncryptedDB(const string& dbPath, const string& key) {
        if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
            throw runtime_error("Can't open database");
        }
        
        // Set encryption key and configure SQLCipher
        string sql = "PRAGMA key = '" + key + "';";
        sql += "PRAGMA cipher_memory_security = ON;";
        sql += "PRAGMA cipher_page_size = 4096;";
        sql += "PRAGMA kdf_iter = 256000;";
        
        if (sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL) != SQLITE_OK) {
            throw runtime_error("Invalid encryption key");
        }
        
        // Create table if not exists
        const char* createSQL = 
            "CREATE TABLE IF NOT EXISTS notes ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "title TEXT NOT NULL,"
            "content TEXT NOT NULL,"
            "modified_date TEXT NOT NULL);";
        
        if (sqlite3_exec(db, createSQL, NULL, NULL, NULL) != SQLITE_OK) {
            throw runtime_error("Can't create table");
        }
    }
    
    ~EncryptedDB() {
        sqlite3_close(db);
    }
    
    // Add a new note
    int addNote(const string& title, const string& content) {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        stringstream ss;
        ss << put_time(ltm, "%Y-%m-%d %H:%M:%S");
        string date = ss.str();
        
        string sql = "INSERT INTO notes (title, content, modified_date) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            return -1;
        }
        
        sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return -1;
        }
        
        int id = sqlite3_last_insert_rowid(db);
        sqlite3_finalize(stmt);
        return id;
    }
    
    // Update an existing note
    bool updateNote(int id, const string& title, const string& content) {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        stringstream ss;
        ss << put_time(ltm, "%Y-%m-%d %H:%M:%S");
        string date = ss.str();
        
        string sql = "UPDATE notes SET title = ?, content = ?, modified_date = ? WHERE id = ?;";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, id);
        
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return success;
    }
    
    // Get all notes for the browser
    vector<vector<string>> getNoteList() {
        vector<vector<string>> notes;
        const char* sql = "SELECT id, title, modified_date FROM notes ORDER BY modified_date DESC;";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
            return notes;
        }
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            vector<string> row;
            row.push_back(to_string(sqlite3_column_int(stmt, 0))); // id
            row.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))); // title
            row.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))); // date
            notes.push_back(row);
        }
        
        sqlite3_finalize(stmt);
        return notes;
    }
    
    // Get a single note by ID
    vector<string> getNote(int id) {
        vector<string> note;
        string sql = "SELECT title, content FROM notes WHERE id = ?;";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            return note;
        }
        
        sqlite3_bind_int(stmt, 1, id);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            note.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))); // title
            note.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))); // content
        }
        
        sqlite3_finalize(stmt);
        return note;
    }
};

// Main application class
class NoteApp {
private:
    Fl_Window* window;
    Fl_Menu_Bar* menu;
    Fl_Browser* browser;
    Fl_Text_Editor* editor;
    Fl_Text_Buffer* textBuffer;
    Fl_Box* statusBar;
    EncryptedDB* db;
    int currentNoteId;
	bool loading; 
    
    // Menu callback
    static void menu_cb(Fl_Widget* w, void* data) {
        NoteApp* app = static_cast<NoteApp*>(data);
        Fl_Menu_Bar* m = static_cast<Fl_Menu_Bar*>(w);
        const Fl_Menu_Item* item = m->mvalue();
        
        if (strcmp(item->label(), "New Note") == 0) {
            app->newNote();
        }
        else if (strcmp(item->label(), "Exit") == 0) {
            app->window->hide();
        } 
    }
    
    // Browser callback
    static void browser_cb(Fl_Widget* w, void* data) {
        NoteApp* app = static_cast<NoteApp*>(data);
        Fl_Browser* b = static_cast<Fl_Browser*>(w);
        
        if (b->value() > 0) {
            // Extract ID from browser line
            string line = b->text(b->value());
            size_t pos = line.find("ID:");
            if (pos != string::npos) {
                int id = stoi(line.substr(pos + 3));
                app->loadNote(id);
            }
        }
    }
    
    // Editor modification callback
    static void modify_cb(int pos, int nInserted, int nDeleted, int, const char*, void* data) {
        NoteApp* app = static_cast<NoteApp*>(data);
        if (!app->loading && app->currentNoteId > 0 && (nInserted > 0 || nDeleted > 0)) {
            app->saveCurrentNote();
        }
    }
    
    // Create a new note
    void newNote() {
        // Create a new note with default title and content
        int id = db->addNote("New Note", "");
        if (id > 0) {
            currentNoteId = id;
            textBuffer->text("");
            updateBrowser();
            browser->select(browser->size()); // Select the new note
            updateStatusBar("Created new note");
        }
    }
    
    // Load a note into the editor
    void loadNote(int id) {
		loading = true;
        vector<string> note = db->getNote(id);
        if (!note.empty()) {
            currentNoteId = id;
            textBuffer->text(note[1].c_str());
            updateStatusBar("Loaded note: " + note[0]);
        }
		loading = false;
    }
    
    // Save the current note
    void saveCurrentNote() {
        if (currentNoteId > 0) {
            string content = textBuffer->text();
            string title = "Note " + to_string(currentNoteId);
            
            // Extract title from first line if possible
            string firstLine = content.substr(0, content.find('\n'));
            if (!firstLine.empty() && firstLine.length() < 50) {
                title = firstLine;
            }
            
            if (db->updateNote(currentNoteId, title, content)) {
                updateBrowser();
                updateStatusBar("Saved: " + title);
            }
        }
    }
    
    // Update the browser list
    void updateBrowser() {
        browser->clear();
        vector<vector<string>> notes = db->getNoteList();
        
        for (const auto& note : notes) {
            // string display = note[0];
            string display = note[1] + " Date: " + note[2] + "   ID:" + note[0];
            browser->add(display.c_str());
            
            // Select the current note
            if (currentNoteId == stoi(note[0])) {
                browser->select(browser->size());
            }
        }
    }
    
    // Update the status bar
    void updateStatusBar(const string& message) {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        stringstream ss;
        ss << put_time(ltm, "%H:%M:%S");
        string time = ss.str();
        
        string status = "[" + time + "] " + message;
        statusBar->copy_label(status.c_str());
    }
    
public:
    NoteApp(int w, int h) : currentNoteId(-1), loading(false) {
        // Create database with encryption
        try {
            db = new EncryptedDB("notes.db", "MyStrongPassword123!");
        } catch (const exception& e) {
            cerr << "Database error: " << e.what() << endl;
            exit(1);
        }
        
        // Create main window
        window = new Fl_Window(w, h, "Encrypted Notes");
        window->color(FL_WHITE);
        
        // Create menu bar
        menu = new Fl_Menu_Bar(0, 0, w, 25);
        menu->add("File/New Note", FL_COMMAND + 'n', menu_cb, this);
        menu->add("File/Exit", FL_COMMAND + 'q', menu_cb, this);
        
        // Create browser for note list
        browser = new Fl_Browser(0, 25, w/3, h - 50);
        browser->type(FL_HOLD_BROWSER);
		browser->has_scrollbar(Fl_Browser_::VERTICAL);

        // browser->textsize(14);
        browser->callback(browser_cb, this);
        browser->color(fl_rgb_color(240, 240, 245));
        
        // Create editor
        textBuffer = new Fl_Text_Buffer();
        editor = new Fl_Text_Editor(w/3, 25, 3*w/4, h - 50);
        editor->buffer(textBuffer);
        editor->textfont(FL_COURIER);
        editor->textsize(16);
        editor->wrap_mode(Fl_Text_Editor::WRAP_AT_BOUNDS, 0);
        
        // Create status bar
        statusBar = new Fl_Box(0, h - 25, w, 25);
        statusBar->box(FL_FLAT_BOX);
        statusBar->color(fl_rgb_color(230, 230, 235));
        statusBar->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        statusBar->labelsize(12);
        
        // Set callbacks
        textBuffer->add_modify_callback(modify_cb, this);
        
        // Load existing notes
        updateBrowser();
        if (browser->size() > 0) {
            browser->select(1);
            browser_cb(browser, this);
        } else {
            newNote();
        }
        
        window->end();
        window->resizable(editor);
    }
    
    ~NoteApp() {
        delete db;
    }
    
    void show() {
        window->show();
    }
};

int main() {
    // Initialize application
    NoteApp app(800, 600);
    app.show();
    
    return Fl::run();
}