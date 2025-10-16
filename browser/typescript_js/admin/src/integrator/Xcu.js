var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
/* -*- js-indent-level: 8 -*- */
/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
var _ = window._;
var defaultXcuObj = {
    Calc: {
        Grid: {
            Option: {
                SnapToGrid: false,
                SizeToGrid: true,
                VisibleGrid: false,
                Synchronize: true,
            },
        },
        Print: {
            Page: {
                EmptyPages: false,
                ForceBreaks: false,
            },
            Other: {
                AllSheets: false,
            },
        },
    },
    Draw: {
        Grid: {
            Option: {
                SnapToGrid: true,
                VisibleGrid: false,
                Synchronize: false,
            },
            SnapGrid: {
                Size: true,
            },
        },
        Print: {
            Content: {
                Drawing: true,
            },
            Page: {
                PageSize: false,
                PageTile: false,
                Booklet: false,
                BookletFront: true,
                BookletBack: true,
            },
            Other: {
                PageName: false,
                Date: false,
                Time: false,
                HiddenPage: true,
                FromPrinterSetup: false,
            },
        },
    },
    Impress: {
        Grid: {
            Option: {
                SnapToGrid: true,
                VisibleGrid: false,
                Synchronize: false,
            },
            SnapGrid: {
                Size: true,
            },
        },
        Print: {
            Content: {
                Presentation: true,
                Note: false,
                Handout: false,
                Outline: false,
            },
            Page: {
                PageSize: false,
                PageTile: false,
                Booklet: false,
                BookletFront: true,
                BookletBack: true,
            },
            Other: {
                PageName: false,
                Date: false,
                Time: false,
                HiddenPage: true,
                FromPrinterSetup: false,
                HandoutHorizontal: false,
            },
        },
    },
    Writer: {
        Grid: {
            Option: {
                SnapToGrid: false,
                VisibleGrid: false,
                Synchronize: false,
            },
        },
        Print: {
            Content: {
                Graphic: true,
                Table: true,
                Drawing: true,
                Control: true,
                Background: true,
                PrintBlack: false,
                PrintHiddenText: false,
                PrintPlaceholders: false,
            },
            Page: {
                LeftPage: true,
                RightPage: true,
                Reversed: false,
                Brochure: false,
                BrochureRightToLeft: false,
            },
            Output: {
                SinglePrintJob: false,
            },
            Papertray: {
                FromPrinterSetup: false,
            },
            EmptyPages: true,
        },
        Content: {
            Display: {
                GraphicObject: true,
            },
        },
    },
};
var Xcu = /** @class */ (function () {
    function Xcu(fileId, XcuFileContent) {
        this.xcuDataObj = {};
        this.fileId = null;
        this.fileId = fileId;
        try {
            this.xcuDataObj =
                XcuFileContent === null || XcuFileContent.length === 0
                    ? defaultXcuObj
                    : this.parse(XcuFileContent);
        }
        catch (error) {
            window.SettingIframe.showErrorModal(_('Something went wrong while loading Document settings.'));
            console.error('Error parsing XCU file:', error);
        }
    }
    Xcu.prototype.parse = function (content) {
        var parser = new DOMParser();
        var xmlDoc = parser.parseFromString(content, 'application/xml');
        if (xmlDoc.getElementsByTagName('parsererror').length > 0) {
            window.SettingIframe.showErrorModal(_('Something went wrong while loading Document settings.'));
        }
        var result = {};
        var items = xmlDoc.getElementsByTagName('item');
        Array.from(items).forEach(function (item) {
            var rawPath = item.getAttribute('oor:path');
            if (!rawPath) {
                return;
            }
            var prefix = '/org.openoffice.Office.';
            var path = rawPath.startsWith(prefix)
                ? rawPath.slice(prefix.length)
                : rawPath;
            var keys = path.split('/').filter(function (key) { return key.trim() !== ''; });
            var currentLevel = result;
            var gridLevel;
            keys.forEach(function (key) {
                if (!(key in currentLevel)) {
                    currentLevel[key] = {};
                }
                currentLevel = currentLevel[key];
                if (key === 'Grid') {
                    gridLevel = currentLevel;
                    gridLevel['ShowGrid'] = false;
                }
            });
            var props = item.getElementsByTagName('prop');
            Array.from(props).forEach(function (prop) {
                var propName = prop.getAttribute('oor:name');
                if (!propName) {
                    return;
                }
                var valueElement = prop.getElementsByTagName('value')[0];
                var value = valueElement
                    ? valueElement.textContent || ''
                    : '';
                var lowerValue = value.toLowerCase();
                if (lowerValue === 'true') {
                    value = true;
                }
                else if (lowerValue === 'false') {
                    value = false;
                }
                if (typeof value === 'boolean') {
                    if (propName === 'VisibleGrid') {
                        gridLevel['ShowGrid'] = value;
                    }
                    else {
                        currentLevel[propName] = value;
                    }
                }
            });
        });
        return result;
    };
    Xcu.prototype.generate = function (xcu) {
        function generateItemNodes(node, path) {
            var items = [];
            var leafProps = {};
            var nestedKeys = [];
            for (var key in node) {
                if (Object.prototype.hasOwnProperty.call(node, key)) {
                    var value = node[key];
                    if (value !== null &&
                        typeof value === 'object' &&
                        !Array.isArray(value)) {
                        nestedKeys.push(key);
                    }
                    else {
                        leafProps[key] = value;
                    }
                }
            }
            var visibleGrid;
            if (Object.keys(leafProps).length > 0) {
                var oorPath = '/org.openoffice.Office.' + path.join('/');
                var propsXml = '';
                for (var propName in leafProps) {
                    if (Object.prototype.hasOwnProperty.call(leafProps, propName)) {
                        if (propName === 'ShowGrid') {
                            visibleGrid = leafProps[propName];
                            continue;
                        }
                        var value = leafProps[propName];
                        var valueStr = '';
                        if (typeof value === 'boolean') {
                            valueStr = value ? 'true' : 'false';
                        }
                        else {
                            valueStr = String(value);
                        }
                        propsXml += "\n    <prop oor:name=\"".concat(propName, "\" oor:op=\"fuse\"><value>").concat(valueStr, "</value></prop>");
                    }
                }
                var itemXml = "  <item oor:path=\"".concat(oorPath, "\">") +
                    propsXml +
                    "\n  </item>";
                items.push(itemXml);
            }
            for (var _i = 0, nestedKeys_1 = nestedKeys; _i < nestedKeys_1.length; _i++) {
                var key = nestedKeys_1[_i];
                var child = node[key];
                if (typeof visibleGrid === 'boolean') {
                    child['VisibleGrid'] = visibleGrid;
                }
                var newPath = path.concat(key);
                items.push.apply(items, generateItemNodes(child, newPath));
            }
            return items;
        }
        var itemsXml = generateItemNodes(xcu, []);
        var xcuXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "<oor:items\n" +
            "    xmlns:oor=\"http://openoffice.org/2001/registry\"\n" +
            "    xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"\n" +
            "    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n\n" +
            itemsXml.join('\n') +
            "\n</oor:items>";
        return xcuXml;
    };
    Xcu.prototype.createXcuEditorUI = function (container) {
        var _this = this;
        var heading = document.createElement('h3');
        heading.textContent = _('Document Settings');
        container.appendChild(heading);
        var descEl = document.createElement('p');
        descEl.textContent = _('Adjust how office documents behave.');
        container.appendChild(descEl);
        var editorContainer = document.createElement('div');
        editorContainer.id = 'xcu-editor';
        var navContainer = document.createElement('div');
        navContainer.className = 'xcu-editor-tabs-nav';
        var tabs = [
            { id: 'calc', label: 'Calc' },
            { id: 'writer', label: 'Writer' },
            { id: 'impress', label: 'Impress' },
            { id: 'draw', label: 'Draw' },
        ];
        tabs.forEach(function (tab) {
            var btn = document.createElement('button');
            btn.type = 'button';
            btn.className = 'xcu-editor-tab';
            btn.id = "xcu-tab-".concat(tab.id);
            btn.textContent = tab.label;
            btn.addEventListener('click', function () {
                navContainer
                    .querySelectorAll('.xcu-editor-tab')
                    .forEach(function (b) { return b.classList.remove('active'); });
                btn.classList.add('active');
                var contentsContainer = editorContainer.querySelector('#xcu-tab-contents');
                contentsContainer.innerHTML = '';
                if (_this.xcuDataObj && _this.xcuDataObj[tab.label]) {
                    var renderedTree = window.settingIframe.renderSettingsOption(_this.xcuDataObj[tab.label]);
                    renderedTree.classList.add('xcu-settings-grid');
                    contentsContainer.appendChild(renderedTree);
                }
                else {
                    contentsContainer.textContent = "No settings available for ".concat(tab.label);
                }
            });
            navContainer.appendChild(btn);
        });
        var contentsContainer = document.createElement('div');
        contentsContainer.id = 'xcu-tab-contents';
        contentsContainer.textContent = _('Select a tab to view settings.');
        editorContainer.appendChild(navContainer);
        editorContainer.appendChild(contentsContainer);
        container.appendChild(editorContainer);
        var actionsContainer = document.createElement('div');
        actionsContainer.classList.add('xcu-editor-actions');
        var resetButton = document.createElement('button');
        resetButton.type = 'button';
        resetButton.id = 'document-settings-reset-button';
        resetButton.classList.add('button', 'button--vue-secondary');
        resetButton.title = _('Reset to default Document settings');
        resetButton.innerHTML = "\n\t\t\t<span class=\"button__wrapper\">\n\t\t\t\t<span class=\"button__icon xcu-reset-icon\">\n\t\t\t\t<svg fill=\"currentColor\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\">\n\t\t\t\t\t<!-- Replace with your Reset icon SVG path -->\n\t\t\t\t\t<path d=\"M12 4V1L8 5l4 4V6c3.31 0 6 2.69 6 6 0 .34-.03.67-.08 1h2.02c.05-.33.06-.66.06-1 0-4.42-3.58-8-8-8zm-6 7c0-.34.03-.67.08-1H4.06c-.05.33-.06.66-.06 1 0 4.42 3.58 8 8 8v3l4-4-4-4v3c-3.31 0-6-2.69-6-6z\"></path>\n\t\t\t\t</svg>\n\t\t\t\t</span>\n\t\t\t</span>\n\t\t\t";
        resetButton.addEventListener('click', function () { return __awaiter(_this, void 0, void 0, function () {
            var confirmed;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        confirmed = window.confirm(_('Are you sure you want to reset Document settings?'));
                        if (!confirmed) {
                            return [2 /*return*/];
                        }
                        resetButton.disabled = true;
                        this.xcuDataObj = defaultXcuObj;
                        return [4 /*yield*/, this.generateXcuAndUpload()];
                    case 1:
                        _a.sent();
                        resetButton.disabled = false;
                        return [2 /*return*/];
                }
            });
        }); });
        var saveButton = document.createElement('button');
        saveButton.type = 'button';
        saveButton.id = 'document-settings-save-button';
        saveButton.classList.add('button', 'button-primary', 'button--text-only');
        saveButton.title = _('Save Document settings');
        var wrapperSpan = document.createElement('span');
        wrapperSpan.classList.add('button__wrapper');
        saveButton.appendChild(wrapperSpan);
        var textSpan = document.createElement('span');
        textSpan.classList.add('button__text');
        textSpan.textContent = _('Save');
        wrapperSpan.appendChild(textSpan);
        saveButton.addEventListener('click', function () { return __awaiter(_this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        saveButton.disabled = true;
                        return [4 /*yield*/, this.generateXcuAndUpload()];
                    case 1:
                        _a.sent();
                        saveButton.disabled = false;
                        return [2 /*return*/];
                }
            });
        }); });
        actionsContainer.appendChild(resetButton);
        actionsContainer.appendChild(saveButton);
        container.appendChild(actionsContainer);
        setTimeout(function () {
            var defaultTab = navContainer.querySelector('#xcu-tab-calc');
            if (defaultTab) {
                defaultTab.click();
            }
        }, 0);
        return container;
    };
    Xcu.prototype.generateXcuAndUpload = function () {
        return __awaiter(this, void 0, void 0, function () {
            var xcuContent;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        xcuContent = this.generate(this.xcuDataObj);
                        return [4 /*yield*/, window.settingIframe.uploadXcuFile(this.fileId, xcuContent)];
                    case 1:
                        _a.sent();
                        return [2 /*return*/];
                }
            });
        });
    };
    return Xcu;
}());
window.Xcu = Xcu;
