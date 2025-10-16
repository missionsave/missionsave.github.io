const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

class EnhancedOutlineProvider {
    constructor(context) {
        this._onDidChangeTreeData = new vscode.EventEmitter();
        this.onDidChangeTreeData = this._onDidChangeTreeData.event;
        this._highlightedItem = null;
        this._currentItems = [];
        this._context = context;
        this._outputChannel = vscode.window.createOutputChannel('Enhanced Outline Debug');

        // 🧠 Store folding state
        this._foldState = this._context.globalState.get('enhancedOutline.foldState', {});
    }

    _saveFoldState() {
        this._context.globalState.update('enhancedOutline.foldState', this._foldState);
    }

    _getFileKey(document) {
        return document?.uri?.toString() || 'unknown';
    }

    _getCollapseState(document, id) {
        const fileKey = this._getFileKey(document);
        return this._foldState[fileKey]?.[id];
    }

    _setCollapseState(document, id, collapsed) {
        const fileKey = this._getFileKey(document);
        if (!this._foldState[fileKey]) this._foldState[fileKey] = {};
        this._foldState[fileKey][id] = collapsed;
        this._saveFoldState();
    }

    showExtensionInfo() {
        try {
            const extensionPath = path.join(this._context.extensionPath, 'extension.js');
            const stats = fs.statSync(extensionPath);
            const modifiedTime = stats.mtime;
            const now = new Date();
            const diffSeconds = Math.round((now - modifiedTime) / 1000);

            this._outputChannel.clear();
            this._outputChannel.appendLine('=== EXTENSION INFO ===');
            this._outputChannel.appendLine(`📂 File: ${path.basename(extensionPath)}`);
            this._outputChannel.appendLine(`📅 Modified: ${modifiedTime.toLocaleString()}`);
            this._outputChannel.appendLine(`🕒 Loaded: ${now.toLocaleString()}`);
            this._outputChannel.appendLine(`⏳ Age: ${diffSeconds} seconds`);
            this._outputChannel.appendLine(diffSeconds > 30 ? '⚠️ WARNING: Possible cached version!' : '✅ Extension up-to-date');
            this._outputChannel.appendLine('======================');
            this._outputChannel.show(true);
        } catch (error) {
            this._outputChannel.appendLine(`❌ Error: ${error.message}`);
        }
    }

    forceReload() {
        this.showExtensionInfo();
        setTimeout(() => vscode.commands.executeCommand('workbench.action.reloadWindow'), 300);
    }

    refresh() {
        this._onDidChangeTreeData.fire();
    }

    getTreeItem(element) {
        if (!element) return new vscode.TreeItem('No elements found');
        element.description = this._highlightedItem === element ? '←' : '';
        return element;
    }

    async getDocumentSymbols(document) {
        try {
            const symbols = await vscode.commands.executeCommand(
                'vscode.executeDocumentSymbolProvider',
                document.uri
            );

            const processSymbol = (symbol) => {
                const cleanName = symbol.name.split(':')[0].trim();
                const processedChildren = symbol.children ? symbol.children.map(processSymbol) : [];
                const range = symbol.range || (
                    symbol.location ? new vscode.Range(
                        new vscode.Position(symbol.location.range.start.line, 0),
                        new vscode.Position(symbol.location.range.end.line, 1000)
                    ) : undefined
                );

                return {
                    ...symbol,
                    name: cleanName,
                    children: processedChildren,
                    range: range
                };
            };

            return Array.isArray(symbols) ? symbols.map(processSymbol) : [];
        } catch (e) {
            console.error('Error getting symbols:', e);
            return [];
        }
    }

    parseRegions(document) {
        const REGION_REGEX = /^\s*#pragma\s+region\s+(.*)$/;
        const ENDREGION_REGEX = /^\s*#pragma\s+endregion\b/;
        const regions = [];
        const stack = [];
        let count = 0;

        for (let line = 0; line < document.lineCount; line++) {
            const text = document.lineAt(line).text;
            const startMatch = text.match(REGION_REGEX);
            if (startMatch) {
                const name = startMatch[1]?.trim() || `Region ${++count}`;
                const region = { 
                    name, 
                    rangeStart: line, 
                    rangeEnd: line, 
                    children: [] 
                };
                if (stack.length > 0) stack[stack.length - 1].children.push(region);
                else regions.push(region);
                stack.push(region);
                continue;
            }
            if (ENDREGION_REGEX.test(text)) {
                if (stack.length > 0) {
                    const region = stack.pop();
                    region.rangeEnd = line;
                }
            }
        }

        const wrap = r => ({
            name: r.name,
            range: new vscode.Range(
                new vscode.Position(r.rangeStart, 0), 
                new vscode.Position(r.rangeEnd, 1000)
            ),
            children: r.children.map(wrap),
            raw: r
        });

        return regions.map(wrap);
    }

    getAllSymbols(symbols) {
        const result = [];
        for (const symbol of symbols) {
            result.push(symbol);
            if (symbol.children) {
                result.push(...this.getAllSymbols(symbol.children));
            }
        }
        return result;
    }

    async getChildren(element) {
        const editor = vscode.window.activeTextEditor;
        if (!editor) return [];

        const document = editor.document;
        const symbols = await this.getDocumentSymbols(document);
        const regions = this.parseRegions(document);
        this._currentSymbols = symbols;

        if (!element) {
            const allItems = [];
            const allSymbols = this.getAllSymbols(symbols);
            for (const symbol of allSymbols) {
                allItems.push({ type: 'symbol', data: symbol, range: symbol.range });
            }
            for (const region of regions) {
                allItems.push({ type: 'region', data: region, range: region.range });
            }

            allItems.sort((a, b) => a.range.start.line - b.range.start.line);

            const rootItems = [];
            const stack = [];

            for (const item of allItems) {
                while (stack.length > 0 && stack[stack.length - 1].range.end.line < item.range.start.line) {
                    stack.pop();
                }

                let treeItem;
                if (item.type === 'symbol') {
                    treeItem = this.createSymbolItem(item.data, document);
                } else {
                    treeItem = this.createRegionItem(item.data, document);
                }

                if (stack.length === 0) {
                    rootItems.push(treeItem);
                } else {
                    const parent = stack[stack.length - 1];
                    if (!parent.children) parent.children = [];
                    parent.children.push(treeItem);
                }

                if (item.range.start.line < item.range.end.line) {
                    stack.push(treeItem);
                }
            }

            this._currentItems = rootItems;
            return rootItems;
        }

        if (element.contextValue === 'region' || element.contextValue === 'symbol') {
            return element.children || [];
        }

        return [];
    }

    createSymbolItem(symbol, document) {
        const id = `symbol:${symbol.name}:${symbol.range.start.line}`;
        const savedState = this._getCollapseState(document, id);
        const hasChildren = symbol.children && symbol.children.length > 0;
        const collapsibleState = hasChildren
            ? (savedState === false
                ? vscode.TreeItemCollapsibleState.Expanded
                : vscode.TreeItemCollapsibleState.Collapsed)
            : vscode.TreeItemCollapsibleState.None;

        const item = new vscode.TreeItem(symbol.name, collapsibleState);
        item.id = id;
        item.iconPath = this.getSymbolIcon(symbol.kind);
        item.contextValue = 'symbol';
        item.range = symbol.range;
        item.symbol = symbol;
        item.command = {
            command: 'enhancedOutline.revealRange',
            title: 'Go to Symbol',
            arguments: [symbol.range]
        };
        return item;
    }

    createRegionItem(region, document) {
        const id = `region:${region.name}:${region.range.start.line}`;
        const savedState = this._getCollapseState(document, id);
        const collapsibleState = (savedState === false)
            ? vscode.TreeItemCollapsibleState.Expanded
            : vscode.TreeItemCollapsibleState.Collapsed;

        const item = new vscode.TreeItem(region.name, collapsibleState);
        item.id = id;
        item.iconPath = new vscode.ThemeIcon('folder');
        item.contextValue = 'region';
        item.range = region.range;
        item.region = region;
        item.command = {
            command: 'enhancedOutline.revealRange',
            title: 'Go to Region',
            arguments: [region.range]
        };
        return item;
    }

    getSymbolIcon(kind) {
        const iconMap = {
            [vscode.SymbolKind.File]: 'file',
            [vscode.SymbolKind.Module]: 'file-submodule',
            [vscode.SymbolKind.Namespace]: 'symbol-namespace',
            [vscode.SymbolKind.Package]: 'package',
            [vscode.SymbolKind.Class]: 'symbol-class',
            [vscode.SymbolKind.Method]: 'symbol-method',
            [vscode.SymbolKind.Property]: 'symbol-property',
            [vscode.SymbolKind.Field]: 'symbol-field',
            [vscode.SymbolKind.Constructor]: 'symbol-constructor',
            [vscode.SymbolKind.Enum]: 'symbol-enum',
            [vscode.SymbolKind.Interface]: 'symbol-interface',
            [vscode.SymbolKind.Function]: 'symbol-function',
            [vscode.SymbolKind.Variable]: 'symbol-variable',
            [vscode.SymbolKind.Constant]: 'symbol-constant',
            [vscode.SymbolKind.String]: 'symbol-string',
            [vscode.SymbolKind.Number]: 'symbol-number',
            [vscode.SymbolKind.Boolean]: 'symbol-boolean',
            [vscode.SymbolKind.Array]: 'symbol-array',
            [vscode.SymbolKind.Object]: 'symbol-object',
            [vscode.SymbolKind.Key]: 'symbol-key',
            [vscode.SymbolKind.Null]: 'symbol-null',
            [vscode.SymbolKind.EnumMember]: 'symbol-enum-member',
            [vscode.SymbolKind.Struct]: 'symbol-structure',
            [vscode.SymbolKind.Event]: 'symbol-event',
            [vscode.SymbolKind.Operator]: 'symbol-operator',
            [vscode.SymbolKind.TypeParameter]: 'symbol-type-parameter'
        };
        return new vscode.ThemeIcon(iconMap[kind] || 'symbol-misc');
    }

    updateHighlightingForPosition(pos) {
        const previous = this._highlightedItem;
        this._highlightedItem = null;

        for (const item of this._currentItems) {
            if (item.range?.contains(pos)) {
                this._highlightedItem = item;
                break;
            }
        }

        if (this._highlightedItem !== previous) {
            this.refresh();
        }
    }
}

function activate(context) {
    const provider = new EnhancedOutlineProvider(context);
    provider.showExtensionInfo();

    const reloadButton = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Right, 100);
    reloadButton.text = '$(debug-restart) Reload';
    reloadButton.tooltip = 'Force reload the extension';
    reloadButton.command = 'enhancedOutline.forceReload';
    reloadButton.show();

    const treeView = vscode.window.createTreeView('enhancedOutline', {
        treeDataProvider: provider,
        showCollapseAll: true
    });

    // 🧩 Capture expand/collapse events
    treeView.onDidCollapseElement(e => {
        const editor = vscode.window.activeTextEditor;
        if (editor) provider._setCollapseState(editor.document, e.element.id, true);
    });
    treeView.onDidExpandElement(e => {
        const editor = vscode.window.activeTextEditor;
        if (editor) provider._setCollapseState(editor.document, e.element.id, false);
    });

    context.subscriptions.push(
        reloadButton,
        treeView,
        vscode.commands.registerCommand('enhancedOutline.forceReload', () => {
            vscode.window.showInformationMessage('Reloading extension...');
            provider.forceReload();
        }),
        vscode.commands.registerCommand('enhancedOutline.showInfo', () => provider.showExtensionInfo()),
        vscode.commands.registerCommand('enhancedOutline.refresh', () => provider.refresh()),
        vscode.commands.registerCommand('enhancedOutline.revealRange', async (range) => {
            const editor = vscode.window.activeTextEditor;
            if (editor && range) {
                editor.selection = new vscode.Selection(range.start, range.start);
                editor.revealRange(range, vscode.TextEditorRevealType.InCenter);
            }
        }),
        vscode.window.onDidChangeTextEditorSelection(e => {
            if (e.textEditor === vscode.window.activeTextEditor && e.selections.length > 0) {
                provider.updateHighlightingForPosition(e.selections[0].active);
            }
        }),
        vscode.workspace.onDidChangeTextDocument(() => provider.refresh()),
        vscode.window.onDidChangeActiveTextEditor(() => provider.refresh())
    );

    setTimeout(() => provider._outputChannel.show(true), 1000);
}

function deactivate() {}

module.exports = { activate, deactivate };
