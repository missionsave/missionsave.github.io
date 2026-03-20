// g++ msvpeek.cpp -o msvpeek $(pkg-config --cflags --libs Qt5Widgets) -lX11 -fPIC
//sudo apt install build-essential qtbase5-dev pkg-config picom
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPainter>
#include <QFileDialog>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QClipboard>
#include <QMimeData>


class CaptureZone : public QWidget {
public:
    CaptureZone(QWidget *parent = nullptr) : QWidget(parent) {
        // Essential: This tells Qt the widget doesn't have a solid background
        setAttribute(Qt::WA_TranslucentBackground);
        setMinimumSize(200, 150);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        // Semi-transparent "viewfinder" tint (Alpha = 30)
        // If you want it COMPLETELY clear, change 30 to 1.
        painter.fillRect(rect(), QColor(0, 100, 255, 1));
        
        // Red border to mark the FFmpeg recording edge
        // painter.setPen(QPen(Qt::red, 2));
        // painter.drawRect(0, 0, width() - 1, height() - 1);
    }
};

class AntiGlitchPeek : public QWidget {
public:
    AntiGlitchPeek() {
        // 1. Make the main window transparent and frameless (optional)
        // If you want to move it easily, keep the frame; 
        // if you want it "floating", add Qt::FramelessWindowHint
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);

        setWindowTitle("OpenGL APNG Recorder");
        resize(640, 520);

        auto *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // 2. The Transparent Area
        captureArea = new CaptureZone(this);
        mainLayout->addWidget(captureArea, 1); 

        // 3. The Opaque Control Bar
        QWidget *controls = new QWidget(this);
        controls->setFixedHeight(60);
        // Setting a stylesheet with a solid color makes ONLY this part opaque
        controls->setStyleSheet("background-color: #1a1a1a; border-top: 1px solid #333;");
        
        auto *btnLayout = new QVBoxLayout(controls);
        recordBtn = new QPushButton("START RECORDING", this);
        recordBtn->setCursor(Qt::PointingHandCursor);
        recordBtn->setStyleSheet(
            "QPushButton { background-color: #27ae60; color: white; font-weight: bold; border-radius: 5px; height: 35px; }"
            "QPushButton:hover { background-color: #2ecc71; }"
        );
        btnLayout->addWidget(recordBtn);

        mainLayout->addWidget(controls);

        connect(recordBtn, &QPushButton::clicked, [this]() {
            this->toggleRecord();
        });
    }

private:
    CaptureZone *captureArea = nullptr;
    QPushButton *recordBtn = nullptr;
    QProcess ffmpeg;
    bool isRecording = false;
    QString tempPath;

    void toggleRecord() {
        if (!isRecording) startRecording();
        else stopRecording();
    }

void startRecording() {
        // tempPath = QDir::tempPath() + "/agif_temp.gif";
        tempPath = QDir::tempPath() + "/apng_temp.apng";
        
        QPoint globalPos = captureArea->mapToGlobal(QPoint(0, 0));
        int x = globalPos.x();
        int y = globalPos.y();
        int w = captureArea->width();
        int h = captureArea->height();

        if (w % 2 != 0) w--;
        if (h % 2 != 0) h--;

        QString display = qgetenv("DISPLAY");
        if (display.isEmpty()) display = ":0.0";

        QStringList args;
        args << "-f" << "x11grab"
             << "-framerate" << "60"
             << "-video_size" << QString("%1x%2").arg(w).arg(h)
             << "-i" << QString("%1+%2,%3").arg(display).arg(x).arg(y)
             // OUTPUT SETTINGS START HERE 
             << "-f" << "apng"
             << "-plays" << "0"           // 0 = Infinite loop
             << "-final_delay" << "1"     // Delay in seconds/100 (3 = 30ms)
             << "-pred" << "1"            // Improves compatibility/compression
             << "-y" << tempPath;


// 		tempPath = QDir::tempPath() + "/temp.gif";

// QStringList args;
// args << "-f" << "x11grab"
//      << "-framerate" << "30"
//      << "-video_size" << QString("%1x%2").arg(w).arg(h)
//      << "-i" << QString("%1+%2,%3").arg(display).arg(x).arg(y)
//      << "-vf" << "fps=30,scale=iw:-1:flags=lanczos"
//      << "-y" << tempPath;


        ffmpeg.setProgram("ffmpeg");
        ffmpeg.setArguments(args);
        ffmpeg.start();

        if (ffmpeg.waitForStarted()) {
            isRecording = true;
            recordBtn->setText("STOP RECORDING (FINALIZE)");
            recordBtn->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; border-radius: 5px; height: 35px;");
        }
    }
void stopRecording() {
    ffmpeg.write("q\n");
    if (!ffmpeg.waitForFinished(10000))
        ffmpeg.kill();

    isRecording = false;
    recordBtn->setText("START RECORDING");
    recordBtn->setStyleSheet("background-color: #27ae60; color: white; font-weight: bold; border-radius: 5px; height: 35px;");

    qDebug() << "Temp path:" << tempPath;
    qDebug() << "Temp exists?" << QFile::exists(tempPath);

    //
    // 1. COPY TEMP FILE TO CLIPBOARD FIRST
    //
    if (QFile::exists(tempPath)) {
{
QClipboard *clipboard = QApplication::clipboard();
QMimeData *mime = new QMimeData();

QFile f(tempPath);
if (f.open(QIODevice::ReadOnly)) {
    QByteArray fileData = f.readAll();
    // mime->setData("image/gif", fileData);
    mime->setData("image/png", fileData);
	mime->setData("image/apng", fileData);
    clipboard->setMimeData(mime);
}


}



 
    } else {
        qDebug() << "ERROR: Temp file does not exist, cannot copy.";
    }

    //
    // 2. SAVE DIALOG
    //
    QString finalPath = QFileDialog::getSaveFileName(
        this,
        "Save APNG",
        "animation.apng",
        "Animated PNG (*.apng)"
    );
// 	QString finalPath = QFileDialog::getSaveFileName(
//     this,
//     "Save GIF",
//     "animation.gif",
//     "GIF Image (*.gif)"
// );


    //
    // 3. SAVE IF USER CHOSE A PATH
    //
    if (!finalPath.isEmpty()) {
        if (QFile::exists(finalPath))
            QFile::remove(finalPath);

        QFile::copy(tempPath, finalPath);
        qDebug() << "Saved to:" << finalPath;
    }

    //
    // 4. REMOVE TEMP FILE
    //
    QFile::remove(tempPath);
}

};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    AntiGlitchPeek window;
    window.show();
    return app.exec();
}