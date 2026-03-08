const vscode = require('vscode');

class EnhancedOutlineProvider {
    constructor(context) {
        this._onDidChangeTreeData = new vscode.EventEmitter();
        this.onDidChangeTreeData = this._onDidChangeTreeData.event;
        this._context = context;
        this.currentRegions = []; 
    }

    refresh() {
        this._onDidChangeTreeData.fire();
    }

    getTreeItem(element) {
        return element;
    }

    // Required for treeView.reveal to work correctly
    getParent(element) {
        return null; // Our list is flat, so no parents
    }

    async getChildren(element) {
        if (element) return []; 
        const editor = vscode.window.activeTextEditor;
        if (!editor) return [];

        this.currentRegions = this.parseRegions(editor.document);
        return this.currentRegions;
    }

    parseRegions(document) {
        const REGION_REGEX = /\/\/region\s+(.*)$/;
        const regions = [];
        
        for (let i = 0; i < document.lineCount; i++) {
            const text = document.lineAt(i).text;
            const match = text.match(REGION_REGEX);

            if (match) {
                const name = match[1].trim() || `Region ${regions.length + 1}`;
                
                if (regions.length > 0) {
                    const prev = regions[regions.length - 1];
                    prev.range = new vscode.Range(
                        prev.range.start,
                        new vscode.Position(i - 1, 1000)
                    );
                }

                const item = new vscode.TreeItem(name, vscode.TreeItemCollapsibleState.None);
                // ID must be unique and stable
                item.id = `region-${i}`; 
                item.iconPath = new vscode.ThemeIcon('symbol-variable');
                item.range = new vscode.Range(new vscode.Position(i, 0), new vscode.Position(document.lineCount - 1, 1000));
                item.command = {
                    command: 'enhancedOutline.revealRange',
                    title: 'Go to Region',
                    arguments: [item.range]
                };

                regions.push(item);
            }
        }
        return regions;
    }
}

function activate(context) {
    const provider = new EnhancedOutlineProvider(context);

    const treeView = vscode.window.createTreeView('enhancedOutline', {
        treeDataProvider: provider,
        showCollapseAll: true
    });

    let lastActiveRegion = null;

    const updateSelection = () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor || provider.currentRegions.length === 0) return;

        const cursorPos = editor.selection.active;
        const activeRegion = provider.currentRegions.find(r => r.range.contains(cursorPos));

        // Only reveal if we actually changed regions to prevent flickering
        if (activeRegion && activeRegion.id !== lastActiveRegion?.id) {
            lastActiveRegion = activeRegion;
            // The 'reveal' call:
            treeView.reveal(activeRegion, {
                select: true,
                focus: false,
                preserveFocus: true
            });
        }
    };

    context.subscriptions.push(
        treeView,
        vscode.commands.registerCommand('enhancedOutline.refresh', () => provider.refresh()),
        vscode.commands.registerCommand('enhancedOutline.revealRange', (range) => {
            const editor = vscode.window.activeTextEditor;
            if (editor && range) {
                editor.selection = new vscode.Selection(range.start, range.start);
                editor.revealRange(range, vscode.TextEditorRevealType.InCenter);
            }
        }),

        // 1. Listen for caret movement
        vscode.window.onDidChangeTextEditorSelection(() => updateSelection()),

        // 2. Listen for file switching
        vscode.window.onDidChangeActiveTextEditor(() => {
            provider.refresh();
            // Wait for the tree to rebuild before trying to select
            setTimeout(updateSelection, 200);
        }),

        // 3. Listen for document changes (typing new regions)
        vscode.workspace.onDidChangeTextDocument(e => {
            if (e.document === vscode.window.activeTextEditor?.document) {
                provider.refresh();
            }
        })
    );
}

exports.activate = activate;