var __assign = (this && this.__assign) || function () {
    __assign = Object.assign || function(t) {
        for (var s, i = 1, n = arguments.length; i < n; i++) {
            s = arguments[i];
            for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p))
                t[p] = s[p];
        }
        return t;
    };
    return __assign.apply(this, arguments);
};
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
var __spreadArray = (this && this.__spreadArray) || function (to, from, pack) {
    if (pack || arguments.length === 2) for (var i = 0, l = from.length, ar; i < l; i++) {
        if (ar || !(i in from)) {
            if (!ar) ar = Array.prototype.slice.call(from, 0, i);
            ar[i] = from[i];
        }
    }
    return to.concat(ar || Array.prototype.slice.call(from));
};
/* eslint-disable */
/* -*- js-indent-level: 8 -*- */
/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
var _ = function (s) { return s.toLocaleString(); };
var initTranslationStr = function () {
    var element = document.getElementById('initial-variables');
    document.documentElement.lang =
        element.dataset.lang || 'en-US';
    String.defaultLocale = 'en-US';
    String.locale =
        document.documentElement.getAttribute('lang') || String.defaultLocale;
};
var onLoaded = function () {
    window.addEventListener('message', onMessage, false);
    window.parent.postMessage('{"MessageId":"settings-ready"}', '*');
};
var onMessage = function (e) {
    try {
        var data = JSON.parse(e.data);
        if (e.origin === window.origin && window.parent !== window.self) {
            if (data.MessageId === 'settings-ready')
                window.parent.postMessage('{"MessageId":"settings-show"}', '*');
            else if (data.MessageId === 'settings-save-all') {
                var saveButtons = [
                    'xcu-save-button',
                    'browser-settings-save-button',
                    'document-settings-save-button',
                ];
                for (var i in saveButtons) {
                    var button = document.getElementById(saveButtons[i]);
                    button === null || button === void 0 ? void 0 : button.click();
                }
            }
        }
    }
    catch (err) {
        console.error('Could not process postmessage:', err);
        return;
    }
};
var defaultBrowserSetting = {
    compactMode: {
        value: false,
        label: 'Compact layout',
        customType: 'compactToggle',
    },
    darkTheme: false,
    spreadsheet: {
        ShowStatusbar: false,
        A11yCheckDeck: false,
        NavigatorDeck: false,
        PropertyDeck: true,
    },
    text: {
        ShowRuler: false,
        ShowStatusbar: false,
        A11yCheckDeck: false,
        NavigatorDeck: false,
        PropertyDeck: true,
        StyleListDeck: false,
    },
    presentation: {
        ShowRuler: false,
        ShowStatusbar: false,
        A11yCheckDeck: false,
        NavigatorDeck: false,
        PropertyDeck: true,
        SdCustomAnimationDeck: false,
        // SdMasterPagesDeck: false,
        // SdSlideTransitionDeck: false,
    },
    drawing: {
        ShowRuler: false,
        ShowStatusbar: false,
        A11yCheckDeck: false,
        NavigatorDeck: false,
        PropertyDeck: true,
    },
};
var SettingIframe = /** @class */ (function () {
    function SettingIframe() {
        var _this = this;
        this._viewSettingLabels = {
            accessibilityState: _('Accessibility'),
            zoteroAPIKey: 'Zotero',
        };
        this.settingLabels = {
            darkTheme: _('Dark Mode'),
            compactMode: _('Compact layout'),
            ShowStatusbar: _('Show status bar'),
            ShowRuler: _('Show Ruler'),
            A11yCheckDeck: _('Accessibility Checker'),
            NavigatorDeck: _('Navigator'),
            PropertyDeck: _('Show Sidebar'),
            SdCustomAnimationDeck: _('Custom Animation'),
            // SdMasterPagesDeck: _('Master Pages'),
            // SdSlideTransitionDeck: _('Slide Transition'),
            StyleListDeck: _('Style List'),
            //Document Settings labels
            Grid: _('Grid'),
            Print: _('Print'),
            Other: _('Other'),
            ShowGrid: _('Show Grid'),
            SnapToGrid: _('Snap to grid'),
            SizeToGrid: _('Size to grid'),
            Synchronize: _('Synchronize axes'),
            SnapGrid: _('Snap grid'),
            EmptyPages: _('Empty Pages'),
            ForceBreaks: _('Force Breaks'),
            AllSheets: _('All Sheets'),
            Size: _('Size to grid'),
            Content: _('Content'),
            Drawing: _('Drawing'),
            Page: _('Page'),
            PageSize: _('Fit to page'),
            PageTile: _('Tile pages'),
            Booklet: _('Booklet'),
            BookletFront: _('Booklet front'),
            BookletBack: _('Booket back'),
            PageName: _('Page name'),
            Date: _('Date'),
            Time: _('Time'),
            HiddenPage: _('Hidden pages'),
            FromPrinterSetup: _('From printer setup'),
            Presentation: _('Presentation'),
            Note: _('Notes'),
            Handout: _('Handouts'),
            Outline: _('Outline'),
            HandoutHorizontal: _('Handout horizontal'),
            Graphic: _('Images'),
            Table: _('Tables'),
            Control: _('Controls'),
            Background: _('Background'),
            PrintBlack: _('Print Black'),
            PrintHiddenText: _('Hidden text'),
            PrintPlaceholders: _('Placeholders'),
            LeftPage: _('Left pages'),
            RightPage: _('Right pages'),
            Brochure: _('Brochure'),
            BrochureRightToLeft: _('Brochure Right to Left'),
            GraphicObject: _('Images and Objects'),
            // Add more as needed
        };
        // SVG templates for icons that are small and always present (no async load needed)
        this.SVG_ICONS = {
            download: "<svg fill=\"currentColor\" width=\"20\" height=\"20\" viewBox=\"0 0 24 24\"><path d=\"M5,20H19V18H5M19,9H15V3H9V9H5L12,16L19,9Z\"></path></svg>",
            delete: "<svg fill=\"currentColor\" width=\"20\" height=\"20\" viewBox=\"0 0 24 24\"><path d=\"M19,4H15.5L14.5,3H9.5L8.5,4H5V6H19M6,19A2,2 0 0,0 8,21H16A2,2 0 0,0 18,19V7H6V19Z\"></path></svg>",
            edit: "<svg fill=\"currentColor\" width=\"20\" height=\"20\" viewBox=\"0 0 24 24\"><path d=\"M3 17.25V21h3.75l11-11.03-3.75-3.75L3 17.25zM20.71 7.04a1 1 0 0 0 0-1.41l-2.34-2.34a1 1 0 0 0-1.41 0l-1.83 1.83 3.75 3.75 1.83-1.83z\"></path></svg>",
            reset: "<svg fill=\"currentColor\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\"><path d=\"M12 4V1L8 5l4 4V6c3.31 0 6 2.69 6 6 0 .34-.03.67-.08 1h2.02c.05-.33.06-.66.06-1 0-4.42-3.58-8-8-8zm-6 7c0-.34.03-.67.08-1H4.06c-.05.33-.06.66-.06 1 0 4.42 3.58 8 8 8v3l4-4-4-4v3c-3.31 0-6-2.69-6-6z\"></path></svg>",
            checkboxMarked: "<svg fill=\"currentColor\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\"><path d=\"M10,17L5,12L6.41,10.58L10,14.17L17.59,6.58L19,8M19,3H5C3.89,3 3,3.89 3,5V19A2,2 0 0,0 5,21H19A2,2 0 0,0 21,19V5C21,3.89 20.1,3 19,3Z\"></path></svg>",
            checkboxBlankOutline: "<svg fill=\"currentColor\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\"><path d=\"M19,3H5C3.89,3 3,3.89 3,5V19A2,2 0 0,0 5,21H19A2,2 0 0,0 21,19V5C21,3.89 20.1,3 19,3M19,5V19H5V5H19Z\"></path></svg>",
        };
        this.PATH = {
            autoTextUpload: function () { return _this.settingConfigBasePath() + '/autotext/'; },
            wordBookUpload: function () { return _this.settingConfigBasePath() + '/wordbook/'; },
            browserSettingsUpload: function () {
                return _this.settingConfigBasePath() + '/browsersetting/';
            },
            viewSettingsUpload: function () {
                return _this.settingConfigBasePath() + '/viewsetting/';
            },
            XcuUpload: function () { return _this.settingConfigBasePath() + '/xcu/'; },
        };
        this.browserSettingOptions = {};
        this.customRenderers = {
            compactToggle: this.renderCompactModeToggle.bind(this),
        };
    }
    SettingIframe.prototype.getAPIEndpoints = function () {
        return {
            uploadSettings: window.serviceRoot + '/browser/dist/upload-settings',
            fetchSharedConfig: window.serviceRoot + '/browser/dist/fetch-settings-config',
            deleteSharedConfig: window.serviceRoot + '/browser/dist/delete-settings-config',
            fetchSettingFile: window.serviceRoot + '/browser/dist/fetch-settings-file',
        };
    };
    SettingIframe.prototype.init = function () {
        this.initWindowVariables();
        this.insertConfigSections();
        void this.fetchAndPopulateSharedConfigs();
        this.wordbook = window.WordBook;
    };
    SettingIframe.prototype.uploadXcuFile = function (filename, content) {
        return __awaiter(this, void 0, void 0, function () {
            var file;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        file = new File([content], filename, {
                            type: 'application/xml',
                        });
                        return [4 /*yield*/, this.uploadFile(this.PATH.XcuUpload(), file)];
                    case 1:
                        _a.sent();
                        return [2 /*return*/];
                }
            });
        });
    };
    SettingIframe.prototype.uploadWordbookFile = function (filename, content) {
        return __awaiter(this, void 0, void 0, function () {
            var file;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        file = new File([content], filename, { type: 'text/plain' });
                        return [4 /*yield*/, this.uploadFile(this.PATH.wordBookUpload(), file)];
                    case 1:
                        _a.sent();
                        return [2 /*return*/];
                }
            });
        });
    };
    SettingIframe.prototype.uploadViewSettingFile = function (filename, content) {
        return __awaiter(this, void 0, void 0, function () {
            var file;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        file = new File([content], filename, { type: 'text/plain' });
                        return [4 /*yield*/, this.uploadFile(this.PATH.viewSettingsUpload(), file)];
                    case 1:
                        _a.sent();
                        return [2 /*return*/];
                }
            });
        });
    };
    SettingIframe.prototype.initWindowVariables = function () {
        var element = document.getElementById('initial-variables');
        if (!element)
            return;
        window.accessToken = element.dataset.accessToken;
        window.accessTokenTTL = element.dataset.accessTokenTtl;
        window.enableDebug = element.dataset.enableDebug === 'true';
        window.enableAccessibility =
            element.dataset.enableAccessibility === 'true';
        window.wopiSettingBaseUrl = element.dataset.wopiSettingBaseUrl;
        window.iframeType = element.dataset.iframeType;
        window.cssVars = element.dataset.cssVars;
        if (window.cssVars) {
            window.cssVars = atob(window.cssVars);
            var sheet = new CSSStyleSheet();
            if (typeof sheet.replace === 'function') {
                sheet.replace(window.cssVars);
                document.adoptedStyleSheets.push(sheet);
            }
        }
        window.serviceRoot = element.dataset.serviceRoot;
        window.versionHash = element.dataset.versionHash;
    };
    SettingIframe.prototype.validateJsonFile = function (file) {
        return new Promise(function (resolve, reject) {
            var reader = new FileReader();
            reader.onload = function (event) {
                var _a;
                try {
                    var content = (_a = event.target) === null || _a === void 0 ? void 0 : _a.result;
                    var jsonData = JSON.parse(content);
                    resolve(jsonData);
                }
                catch (error) {
                    reject(new Error(_('Invalid JSON file')));
                }
            };
            reader.onerror = function () {
                reject(new Error(_('Error reading file')));
            };
            reader.readAsText(file);
        });
    };
    SettingIframe.prototype.insertConfigSections = function () {
        var _this = this;
        var sharedConfigsContainer = document.getElementById('allConfigSection');
        if (!sharedConfigsContainer)
            return;
        var configSections = [
            {
                sectionTitle: _('Autotext'),
                sectionDesc: _('Upload reusable text snippets (.bau). To insert the text in your document, type the shortcut for an AutoText entry and press F3.'),
                listId: 'autotextList',
                inputId: 'autotextFile',
                buttonId: 'uploadAutotextButton',
                fileAccept: '.bau',
                buttonText: _('Upload Autotext'),
                uploadPath: this.PATH.autoTextUpload(),
            },
            {
                sectionTitle: _('Custom dictionaries'),
                sectionDesc: _('Add or edit words in a spell check dictionary. Words in your wordbook (.dic) will be available for spelling checks.'),
                listId: 'wordbookList',
                inputId: 'wordbookFile',
                buttonId: 'uploadWordbookButton',
                fileAccept: '.dic',
                buttonText: _('Upload Wordbook'),
                uploadPath: this.PATH.wordBookUpload(),
            },
            {
                sectionTitle: _('Document settings'),
                sectionDesc: _('Adjust how office documents behave.'),
                listId: 'XcuList',
                inputId: 'XcuFile',
                buttonId: 'uploadXcuButton',
                fileAccept: '.xcu',
                // TODO: replace btn with rich interface (toggles)
                buttonText: _('Upload Xcu'),
                uploadPath: this.PATH.XcuUpload(),
                debugOnly: true,
            },
        ];
        configSections.forEach(function (cfg) {
            if (cfg.enabledFor && cfg.enabledFor !== _this.getConfigType()) {
                return;
            }
            if (cfg.debugOnly && !window.enableDebug) {
                return;
            }
            var sectionEl = _this.createConfigSection(cfg);
            var fileInput = sectionEl.querySelector("#".concat(cfg.inputId));
            var button = sectionEl.querySelector("#".concat(cfg.buttonId));
            if (fileInput && button) {
                button.addEventListener('click', function () {
                    fileInput.click();
                });
                fileInput.addEventListener('change', function () { return __awaiter(_this, void 0, void 0, function () {
                    var file;
                    var _a;
                    return __generator(this, function (_b) {
                        if ((_a = fileInput.files) === null || _a === void 0 ? void 0 : _a.length) {
                            if (cfg.uploadPath === this.PATH.wordBookUpload()) {
                                this.wordbook.wordbookValidation(cfg.uploadPath, fileInput.files[0]);
                            }
                            else {
                                file = fileInput.files[0];
                                this.uploadFile(cfg.uploadPath, file);
                            }
                            fileInput.value = '';
                        }
                        return [2 /*return*/];
                    });
                }); });
            }
            sharedConfigsContainer.appendChild(sectionEl);
        });
    };
    SettingIframe.prototype.fetchAndPopulateSharedConfigs = function () {
        return __awaiter(this, void 0, void 0, function () {
            var formData, response, data, error_1;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        if (!window.wopiSettingBaseUrl) {
                            console.error(_('Shared Config URL is missing in initial variables.'));
                            return [2 /*return*/];
                        }
                        console.debug('iframeType page', window.iframeType);
                        if (!window.accessToken) {
                            console.error(_('Access token is missing in initial variables.'));
                            return [2 /*return*/];
                        }
                        formData = new FormData();
                        formData.append('sharedConfigUrl', window.wopiSettingBaseUrl);
                        formData.append('accessToken', window.accessToken);
                        formData.append('type', this.getConfigType());
                        _a.label = 1;
                    case 1:
                        _a.trys.push([1, 5, , 6]);
                        return [4 /*yield*/, fetch(this.getAPIEndpoints().fetchSharedConfig, {
                                method: 'POST',
                                headers: {
                                    Authorization: "Bearer ".concat(window.accessToken),
                                },
                                body: formData,
                            })];
                    case 2:
                        response = _a.sent();
                        if (!response.ok) {
                            console.error('something went wrong shared config response', response.text());
                            throw new Error("Could not fetch shared config: ".concat(response.statusText));
                        }
                        return [4 /*yield*/, response.json()];
                    case 3:
                        data = _a.sent();
                        return [4 /*yield*/, this.populateSharedConfigUI(data)];
                    case 4:
                        _a.sent();
                        console.debug('Shared config data: ', data);
                        return [3 /*break*/, 6];
                    case 5:
                        error_1 = _a.sent();
                        SettingIframe.showErrorModal(_('Something went wrong. Please try to refresh the page.'));
                        console.error('Error fetching shared config:', error_1);
                        return [3 /*break*/, 6];
                    case 6: return [2 /*return*/];
                }
            });
        });
    };
    SettingIframe.prototype.createConfigSection = function (config) {
        var sectionEl = document.createElement('div');
        sectionEl.classList.add('section');
        sectionEl.appendChild(this.createHeading(config.sectionTitle, 'h3'));
        sectionEl.appendChild(this.createParagraph(config.sectionDesc));
        sectionEl.appendChild(this.createUnorderedList(config.listId));
        sectionEl.appendChild(this.createFileInput(config.inputId, config.fileAccept));
        sectionEl.appendChild(this.createButton(config.buttonId, config.buttonText));
        return sectionEl;
    };
    SettingIframe.prototype.createHeading = function (text, level) {
        if (level === void 0) { level = 'h3'; }
        var headingEl = document.createElement(level);
        headingEl.textContent = text;
        return headingEl;
    };
    SettingIframe.prototype.createParagraph = function (text) {
        var pEl = document.createElement('p');
        pEl.textContent = text;
        return pEl;
    };
    SettingIframe.prototype.createUnorderedList = function (id) {
        var ulEl = document.createElement('ul');
        ulEl.id = id;
        return ulEl;
    };
    SettingIframe.prototype.createFileInput = function (id, accept) {
        var inputEl = document.createElement('input');
        inputEl.type = 'file';
        inputEl.classList.add('hidden');
        inputEl.id = id;
        inputEl.accept = accept;
        return inputEl;
    };
    SettingIframe.prototype.createTextInput = function (id, placeholder, text, onChangeHandler) {
        if (placeholder === void 0) { placeholder = ''; }
        if (text === void 0) { text = ''; }
        if (onChangeHandler === void 0) { onChangeHandler = function (input) { }; }
        var inputEl = document.createElement('input');
        inputEl.type = 'text';
        inputEl.id = id;
        inputEl.value = text;
        inputEl.placeholder = placeholder;
        inputEl.classList.add('dic-input-container');
        inputEl.addEventListener('change', function () {
            onChangeHandler(inputEl);
        });
        return inputEl;
    };
    SettingIframe.prototype.createButton = function (id, text) {
        var buttonEl = document.createElement('button');
        buttonEl.id = id;
        buttonEl.type = 'button';
        buttonEl.classList.add('inline-button', 'button', 'button--text-only', 'button--vue-secondary');
        var wrapperSpan = document.createElement('span');
        wrapperSpan.classList.add('button__wrapper');
        var textSpan = document.createElement('span');
        textSpan.classList.add('button__text');
        textSpan.textContent = text; // Safely set text content
        wrapperSpan.appendChild(textSpan);
        buttonEl.appendChild(wrapperSpan);
        return buttonEl;
    };
    SettingIframe.prototype.fetchSettingFile = function (fileId) {
        return __awaiter(this, void 0, void 0, function () {
            var formData, apiUrl, response, error_2;
            var _a;
            return __generator(this, function (_b) {
                switch (_b.label) {
                    case 0:
                        _b.trys.push([0, 3, , 4]);
                        formData = new FormData();
                        formData.append('fileUrl', fileId);
                        formData.append('accessToken', (_a = window.accessToken) !== null && _a !== void 0 ? _a : '');
                        apiUrl = this.getAPIEndpoints().fetchSettingFile;
                        return [4 /*yield*/, fetch(apiUrl, {
                                method: 'POST',
                                headers: {
                                    Authorization: "Bearer ".concat(window.accessToken),
                                },
                                body: formData,
                            })];
                    case 1:
                        response = _b.sent();
                        if (!response.ok) {
                            throw new Error("Upload failed: ".concat(response.statusText));
                        }
                        return [4 /*yield*/, response.text()];
                    case 2: return [2 /*return*/, _b.sent()];
                    case 3:
                        error_2 = _b.sent();
                        SettingIframe.showErrorModal(_('Something went wrong while fetching setting file. Please try to refresh the page.'));
                        return [2 /*return*/, null];
                    case 4: return [2 /*return*/];
                }
            });
        });
    };
    SettingIframe.prototype.fetchWordbookFile = function (fileId) {
        return __awaiter(this, void 0, void 0, function () {
            var textValue, wordbook, fileName, error_3, message;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        this.wordbook.startLoader();
                        _a.label = 1;
                    case 1:
                        _a.trys.push([1, 4, , 5]);
                        return [4 /*yield*/, this.fetchSettingFile(fileId)];
                    case 2:
                        textValue = _a.sent();
                        if (!textValue) {
                            throw new Error('Failed to fetch wordbook file');
                        }
                        return [4 /*yield*/, this.wordbook.parseWordbookFileAsync(textValue)];
                    case 3:
                        wordbook = _a.sent();
                        fileName = this.getFilename(fileId, false);
                        this.wordbook.stopLoader();
                        this.wordbook.openWordbookEditor(fileName, wordbook);
                        return [3 /*break*/, 5];
                    case 4:
                        error_3 = _a.sent();
                        message = error_3 instanceof Error ? error_3.message : 'Unknown error';
                        console.error("Error uploading file: ".concat(message));
                        SettingIframe.showErrorModal(_('Something went wrong while fetching wordbook. Please try to refresh the page.'));
                        this.wordbook.stopLoader();
                        return [3 /*break*/, 5];
                    case 5: return [2 /*return*/];
                }
            });
        });
    };
    SettingIframe.prototype.createBrowserSettingForm = function (sharedConfigsContainer) {
        var editorContainer = document.createElement('div');
        editorContainer.id = 'browser-setting';
        editorContainer.className = 'section';
        editorContainer.appendChild(this.createHeading(_('Interface Settings')));
        editorContainer.appendChild(this.createParagraph(_('Set default interface preferences.')));
        var navContainer = this.createBrowserSettingTabsNav(editorContainer);
        var commonTogglesData = {};
        for (var _i = 0, _a = Object.entries(this.browserSettingOptions); _i < _a.length; _i++) {
            var _b = _a[_i], key = _b[0], value = _b[1];
            // Include:
            // - plain booleans
            // - objects that have a customType (like compactToggle)
            if (typeof value === 'boolean' ||
                (typeof value === 'object' &&
                    value !== null &&
                    'customType' in value)) {
                commonTogglesData[key] = value;
            }
        }
        if (Object.keys(commonTogglesData).length > 0) {
            var commonTogglesElement = this.renderSettingsOption(commonTogglesData, 'common');
            editorContainer.appendChild(commonTogglesElement);
            var separator = document.createElement('hr');
            separator.style.border = 'none';
            separator.style.borderTop = '1px solid var(--settings-border)';
            separator.style.marginTop = '1rem';
            editorContainer.appendChild(separator);
        }
        var contentsContainer = this.createBrowserSettingContentsContainer();
        var actionsContainer = this.createBrowserSettingActions(sharedConfigsContainer);
        editorContainer.appendChild(navContainer);
        editorContainer.appendChild(contentsContainer);
        editorContainer.appendChild(actionsContainer);
        var oldEditor = sharedConfigsContainer.querySelector('#browser-setting');
        if (oldEditor && oldEditor.parentNode === sharedConfigsContainer) {
            sharedConfigsContainer.replaceChild(editorContainer, oldEditor);
        }
        else {
            sharedConfigsContainer.appendChild(editorContainer);
        }
        setTimeout(function () {
            var defaultTab = navContainer.querySelector('#bs-tab-spreadsheet');
            if (defaultTab) {
                defaultTab.click();
            }
        }, 0);
    };
    SettingIframe.prototype.createBrowserSettingTabsNav = function (editorContainer) {
        var _this = this;
        var navContainer = document.createElement('div');
        navContainer.className = 'browser-setting-tabs-nav';
        var tabs = [
            { id: 'spreadsheet', label: 'Calc' },
            { id: 'text', label: 'Writer' },
            { id: 'presentation', label: 'Impress' },
            { id: 'drawing', label: 'Draw' },
        ];
        tabs.forEach(function (tab) {
            var btn = document.createElement('button');
            btn.type = 'button';
            btn.className = "browser-setting-tab";
            btn.id = "bs-tab-".concat(tab.id);
            btn.textContent = tab.label;
            btn.addEventListener('click', function () {
                navContainer
                    .querySelectorAll('.browser-setting-tab')
                    .forEach(function (b) { return b.classList.remove('active'); });
                btn.classList.add('active');
                var contentsContainer = editorContainer.querySelector('#tab-contents-browserSetting');
                contentsContainer.innerHTML = '';
                if (_this.browserSettingOptions &&
                    _this.browserSettingOptions[tab.id]) {
                    var renderedTree = _this.renderSettingsOption(_this.browserSettingOptions[tab.id], tab.id);
                    renderedTree.classList.add('browser-settings-grid');
                    contentsContainer.appendChild(renderedTree);
                }
                else {
                    contentsContainer.textContent = _("No settings available for ".concat(tab.label));
                }
            });
            navContainer.appendChild(btn);
        });
        return navContainer;
    };
    SettingIframe.prototype.createBrowserSettingContentsContainer = function () {
        var contentsContainer = document.createElement('div');
        contentsContainer.id = 'tab-contents-browserSetting';
        contentsContainer.textContent = _('Select a tab to browser settings.');
        return contentsContainer;
    };
    SettingIframe.prototype.createBrowserSettingActions = function (sharedConfigsContainer) {
        var _this = this;
        var actionsContainer = document.createElement('div');
        actionsContainer.classList.add('browser-settings-editor-actions');
        var resetButton = this.createButtonWithIcon('browser-settings-reset-button', 'reset', // Use icon key
        _('Reset to default Document settings'), ['button--vue-secondary', 'xcu-reset-icon'], function (button) { return __awaiter(_this, void 0, void 0, function () {
            var confirmed;
            return __generator(this, function (_a) {
                confirmed = window.confirm(_('Are you sure you want to reset Document settings?'));
                if (!confirmed) {
                    return [2 /*return*/];
                }
                this.browserSettingOptions = JSON.parse(JSON.stringify(defaultBrowserSetting));
                this.createBrowserSettingForm(sharedConfigsContainer);
                return [2 /*return*/];
            });
        }); }, true);
        actionsContainer.appendChild(resetButton);
        var saveButton = this.createButtonWithText('browser-settings-save-button', _('Save'), _('Save Document settings'), ['button-primary'], function (button) { return __awaiter(_this, void 0, void 0, function () {
            var file;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        button.disabled = true;
                        this.collectBrowserSettingsFromUI(sharedConfigsContainer.querySelector('#browser-setting'));
                        file = new File([JSON.stringify(this.browserSettingOptions)], 'browsersetting.json', {
                            type: 'application/json',
                            lastModified: Date.now(),
                        });
                        return [4 /*yield*/, this.uploadFile(this.PATH.browserSettingsUpload(), file)];
                    case 1:
                        _a.sent();
                        button.disabled = false;
                        return [2 /*return*/];
                }
            });
        }); });
        actionsContainer.appendChild(saveButton);
        return actionsContainer;
    };
    SettingIframe.prototype.createMaterialDesignIconContainer = function (iconSvgString) {
        var materialIconContainer = document.createElement('span');
        materialIconContainer.setAttribute('aria-hidden', 'true');
        materialIconContainer.setAttribute('role', 'img'); // Add role for accessibility where appropriate
        materialIconContainer.classList.add('material-design-icon');
        materialIconContainer.innerHTML = iconSvgString; // Safe as it's from trusted SVG_ICONS
        return materialIconContainer;
    };
    SettingIframe.prototype.createButtonWithIcon = function (id, iconKey, // Use a type-safe key
    title, classes, onClickHandler, isIconOnly) {
        var _a;
        if (isIconOnly === void 0) { isIconOnly = false; }
        var buttonEl = document.createElement('button');
        if (id) {
            buttonEl.id = id;
        }
        buttonEl.type = 'button';
        (_a = buttonEl.classList).add.apply(_a, __spreadArray(['button'], classes, false));
        if (isIconOnly) {
            buttonEl.classList.add('button--icon-only');
        }
        else {
            buttonEl.classList.add('button--text-only');
        }
        buttonEl.title = title;
        var wrapperSpan = document.createElement('span');
        wrapperSpan.classList.add('button__wrapper');
        buttonEl.appendChild(wrapperSpan);
        var iconSpan = document.createElement('span');
        iconSpan.setAttribute('aria-hidden', 'true');
        iconSpan.classList.add('button__icon');
        wrapperSpan.appendChild(iconSpan);
        // Now correctly creates the inner span and injects the SVG
        iconSpan.appendChild(this.createMaterialDesignIconContainer(this.SVG_ICONS[iconKey]));
        if (!isIconOnly) {
            var textSpan = document.createElement('span');
            textSpan.classList.add('button__text');
            textSpan.textContent = title;
            wrapperSpan.appendChild(textSpan);
        }
        buttonEl.addEventListener('click', function () { return onClickHandler(buttonEl); });
        return buttonEl;
    };
    SettingIframe.prototype.createButtonWithText = function (id, text, title, classes, onClickHandler) {
        var _a;
        var buttonEl = document.createElement('button');
        if (id) {
            buttonEl.id = id;
        }
        buttonEl.type = 'button';
        (_a = buttonEl.classList).add.apply(_a, __spreadArray(['button', 'button--text-only'], classes, false));
        buttonEl.title = title;
        var wrapperSpan = document.createElement('span');
        wrapperSpan.classList.add('button__wrapper');
        buttonEl.appendChild(wrapperSpan);
        var textSpan = document.createElement('span');
        textSpan.classList.add('button__text');
        textSpan.textContent = text;
        wrapperSpan.appendChild(textSpan);
        buttonEl.addEventListener('click', function () { return onClickHandler(buttonEl); });
        return buttonEl;
    };
    SettingIframe.prototype.renderSettingsOption = function (data, pathPrefix) {
        if (pathPrefix === void 0) { pathPrefix = ''; }
        var container = document.createElement('div');
        if (typeof data !== 'object' || data === null) {
            container.textContent = String(data);
            return container;
        }
        for (var key in data) {
            if (Object.prototype.hasOwnProperty.call(data, key)) {
                var value = data[key];
                var uniqueId = pathPrefix ? "".concat(pathPrefix, "-").concat(key) : key;
                if (typeof value === 'object' &&
                    (value === null || value === void 0 ? void 0 : value.customType) &&
                    this.customRenderers[value.customType]) {
                    var customElement = this.customRenderers[value.customType](key, value, uniqueId);
                    container.appendChild(customElement);
                    continue;
                }
                if (typeof value === 'object' &&
                    value !== null &&
                    !Array.isArray(value)) {
                    container.appendChild(this.createFieldset(key, value, uniqueId));
                }
                else {
                    container.appendChild(this.createCheckboxToggle(key, value, uniqueId, data));
                }
            }
        }
        return container;
    };
    SettingIframe.prototype.createFieldset = function (key, value, uniqueId) {
        var fieldset = document.createElement('fieldset');
        fieldset.classList.add('xcu-settings-fieldset');
        if (uniqueId.startsWith('Grid-')) {
            fieldset.classList.add('grid-options-fieldset');
        }
        var legend = document.createElement('legend');
        legend.textContent = this.settingLabels[key] || key;
        fieldset.appendChild(legend);
        var childContent = this.renderSettingsOption(value, uniqueId);
        fieldset.appendChild(childContent);
        return fieldset;
    };
    // Helper to create a checkbox input element.
    SettingIframe.prototype.createCheckboxInput = function (id, isChecked, isDisabled) {
        var inputCheckbox = document.createElement('input');
        inputCheckbox.type = 'checkbox';
        inputCheckbox.className = 'checkbox-radio-switch-input';
        inputCheckbox.id = id + '-input';
        inputCheckbox.checked = isChecked;
        inputCheckbox.disabled = isDisabled;
        return inputCheckbox;
    };
    SettingIframe.prototype.createCheckbox = function (id, isChecked, labelText, onClickHandler, isDisabled, warningText) {
        var _this = this;
        if (isDisabled === void 0) { isDisabled = false; }
        if (warningText === void 0) { warningText = null; }
        var checkboxWrapper = document.createElement('span');
        checkboxWrapper.className = "checkbox-radio-switch checkbox-radio-switch-checkbox ".concat(isChecked ? '' : 'checkbox-radio-switch--checked', " checkbox-wrapper");
        id = id.replace(/\s/g, '');
        checkboxWrapper.id = id + '-container';
        // Use the new helper here
        var inputCheckbox = this.createCheckboxInput(id, isChecked, isDisabled);
        checkboxWrapper.appendChild(inputCheckbox);
        var checkboxContent = document.createElement('span');
        checkboxContent.className =
            'checkbox-content checkbox-content-checkbox checkbox-content--has-text checkbox-radio-switch__content';
        checkboxContent.id = id + '-content';
        checkboxWrapper.appendChild(checkboxContent);
        var checkboxContentIcon = document.createElement('span');
        checkboxContentIcon.className = "".concat(isDisabled ? 'checkbox-content-icon-disabled' : 'checkbox-content-icon', " checkbox-content-icon checkbox-radio-switch__icon ").concat(isChecked ? '' : 'checkbox-content-icon--checked');
        checkboxContentIcon.ariaHidden = 'true';
        checkboxContent.appendChild(checkboxContentIcon);
        var materialIconContainer = this.createMaterialDesignIconContainer(isChecked
            ? this.SVG_ICONS.checkboxMarked
            : this.SVG_ICONS.checkboxBlankOutline);
        checkboxContentIcon.appendChild(materialIconContainer);
        var textElement = document.createElement('span');
        textElement.className =
            'checkbox-content__text checkbox-radio-switch__text';
        textElement.textContent = labelText;
        checkboxContent.appendChild(textElement);
        if (warningText) {
            var warningEl = document.createElement('span');
            warningEl.className = 'ui-state-error-text';
            warningEl.textContent = warningText;
            checkboxContent.appendChild(warningEl);
        }
        if (!isDisabled) {
            checkboxWrapper.addEventListener('click', function () {
                onClickHandler(inputCheckbox, checkboxWrapper, materialIconContainer);
                if (checkboxWrapper.id === 'Grid-ShowGrid-container') {
                    _this.toggleGridOptionsVisibility(checkboxWrapper);
                }
            });
            if (checkboxWrapper.id === 'Grid-ShowGrid-container') {
                // Set the initial state of Grid fieldsets' visibility
                setTimeout(function () {
                    return _this.toggleGridOptionsVisibility(checkboxWrapper);
                }, 0);
            }
        }
        else {
            checkboxWrapper.classList.add('checkbox-radio-switch--disabled');
        }
        return checkboxWrapper;
    };
    SettingIframe.prototype.createCheckboxToggle = function (key, value, uniqueId, data) {
        var _this = this;
        var labelText = this.settingLabels[key] || key;
        return this.createCheckbox(uniqueId, value, labelText, function (inputCheckbox, checkboxWrapper, materialIconContainer) {
            var currentChecked = !inputCheckbox.checked;
            inputCheckbox.checked = currentChecked;
            checkboxWrapper.classList.toggle('checkbox-radio-switch--checked', !currentChecked);
            materialIconContainer.innerHTML = currentChecked
                ? _this.SVG_ICONS.checkboxMarked
                : _this.SVG_ICONS.checkboxBlankOutline;
            data[key] = currentChecked;
        });
    };
    SettingIframe.prototype.collectBrowserSettingsFromUI = function (browserSettingSection) {
        var _this = this;
        var inputs = browserSettingSection.querySelectorAll('input.checkbox-radio-switch-input');
        inputs.forEach(function (input) {
            // Expected ID: section-setting-input (e.g., "writer-ShowSidebar-input")
            var parts = input.id.split('-');
            if (parts.length !== 3 || parts[2] !== 'input')
                return;
            var sectionRaw = parts[0], settingKey = parts[1];
            var value = input.checked;
            if (sectionRaw === 'common') {
                _this.browserSettingOptions[settingKey] = value;
            }
            else {
                _this.browserSettingOptions[sectionRaw][settingKey] = value;
            }
        });
    };
    SettingIframe.prototype.renderCompactModeToggle = function (key, setting, uniqueId) {
        var container = document.createElement('div');
        container.className = 'custom-compact-toggle';
        var inputCheckbox = document.createElement('input');
        inputCheckbox.type = 'checkbox';
        inputCheckbox.className = 'checkbox-radio-switch-input';
        inputCheckbox.id = uniqueId + '-input';
        inputCheckbox.checked = setting.value;
        inputCheckbox.style.display = 'none'; // hidden input for logic
        container.appendChild(inputCheckbox);
        var options = document.createElement('div');
        options.className = 'toggle-options';
        var select = function (useCompact) {
            inputCheckbox.checked = useCompact;
            setting.value = useCompact;
            notebookImage.classList.toggle('selected', !useCompact);
            compactImage.classList.toggle('selected', useCompact);
        };
        var notebookOption = this.createCompactToggleOption('Notebookbar.svg', 'Notebookbar', _('Notebookbar view'), !setting.value, function () { return select(false); });
        var compactOption = this.createCompactToggleOption('Compact.svg', 'Compact', _('Compact view'), setting.value, function () { return select(true); });
        var notebookImage = notebookOption.querySelector('.toggle-image');
        var compactImage = compactOption.querySelector('.toggle-image');
        options.appendChild(notebookOption);
        options.appendChild(compactOption);
        container.appendChild(options);
        return container;
    };
    SettingIframe.prototype.createCompactToggleOption = function (imageSrc, imageAlt, labelText, isSelected, onClick) {
        var optionDiv = document.createElement('div');
        optionDiv.className = 'toggle-option';
        var image = document.createElement('img');
        image.src = "".concat(window.serviceRoot, "/browser/").concat(window.versionHash, "/admin/images/").concat(imageSrc);
        image.alt = imageAlt;
        image.className = "toggle-image ".concat(isSelected ? 'selected' : '');
        optionDiv.appendChild(image);
        var label = document.createElement('div');
        label.textContent = labelText;
        label.className = 'toggle-image-label';
        optionDiv.appendChild(label);
        image.addEventListener('click', onClick);
        return optionDiv;
    };
    SettingIframe.prototype.uploadFile = function (filePath, file) {
        return __awaiter(this, void 0, void 0, function () {
            var formData, apiUrl, response, error_4, message;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        formData = new FormData();
                        formData.append('file', file);
                        formData.append('filePath', filePath);
                        if (window.wopiSettingBaseUrl) {
                            formData.append('wopiSettingBaseUrl', window.wopiSettingBaseUrl);
                        }
                        _a.label = 1;
                    case 1:
                        _a.trys.push([1, 4, , 5]);
                        apiUrl = this.getAPIEndpoints().uploadSettings;
                        return [4 /*yield*/, fetch(apiUrl, {
                                method: 'POST',
                                headers: {
                                    Authorization: "Bearer ".concat(window.accessToken),
                                },
                                body: formData,
                            })];
                    case 2:
                        response = _a.sent();
                        if (!response.ok) {
                            throw new Error("Upload failed: ".concat(response.statusText));
                        }
                        return [4 /*yield*/, this.fetchAndPopulateSharedConfigs()];
                    case 3:
                        _a.sent();
                        return [3 /*break*/, 5];
                    case 4:
                        error_4 = _a.sent();
                        message = error_4 instanceof Error ? error_4.message : 'Unknown error';
                        console.error("Error uploading file: ".concat(message));
                        SettingIframe.showErrorModal(_('Something went wrong while uploading the file. Please try again.'));
                        return [3 /*break*/, 5];
                    case 5: return [2 /*return*/];
                }
            });
        });
    };
    SettingIframe.prototype.populateList = function (listId, items, category) {
        var _this = this;
        var listEl = document.getElementById(listId);
        if (!listEl)
            return;
        listEl.innerHTML = '';
        items.forEach(function (item) {
            var fileName = _this.getFilename(item.uri, false);
            var li = document.createElement('li');
            li.classList.add('list-item__wrapper');
            var listItemDiv = document.createElement('div');
            listItemDiv.classList.add('list-item');
            listItemDiv.appendChild(_this.createListItemAnchor(fileName));
            listItemDiv.appendChild(_this.createListItemActions(item, category, fileName));
            li.appendChild(listItemDiv);
            listEl.appendChild(li);
        });
    };
    SettingIframe.prototype.createListItemAnchor = function (fileName) {
        var anchor = document.createElement('div');
        anchor.classList.add('list-item__anchor');
        var listItemContentDiv = document.createElement('div');
        listItemContentDiv.classList.add('list-item-content');
        var listItemContentMainDiv = document.createElement('div');
        listItemContentMainDiv.classList.add('list-item-content__main');
        var listItemContentNameDiv = document.createElement('div');
        listItemContentNameDiv.classList.add('list-item-content__name');
        listItemContentNameDiv.textContent = fileName;
        listItemContentMainDiv.appendChild(listItemContentNameDiv);
        listItemContentDiv.appendChild(listItemContentMainDiv);
        anchor.appendChild(listItemContentDiv);
        return anchor;
    };
    SettingIframe.prototype.createListItemActions = function (item, category, fileName) {
        var _this = this;
        var extraActionsDiv = document.createElement('div');
        extraActionsDiv.classList.add('list-item-content__extra-actions');
        extraActionsDiv.appendChild(this.createButtonWithIcon('', // No specific ID needed for list item buttons
        'download', // Use icon key
        item.uri, // Use URI as title for download link
        ['button--vue-secondary', 'download-icon'], function (button) { return window.open(item.uri, '_blank'); }, true));
        extraActionsDiv.appendChild(this.createButtonWithIcon('', 'delete', // Use icon key
        _('Delete'), ['button--vue-secondary', 'delete-icon'], function (button) { return __awaiter(_this, void 0, void 0, function () {
            var fileId, formData, response, error_5;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        _a.trys.push([0, 3, , 4]);
                        if (!window.accessToken) {
                            throw new Error('Access token is missing.');
                        }
                        if (!window.wopiSettingBaseUrl) {
                            throw new Error('wopiSettingBaseUrl is missing.');
                        }
                        fileId = this.settingConfigBasePath() +
                            category +
                            '/' +
                            fileName;
                        formData = new FormData();
                        formData.append('fileId', fileId);
                        formData.append('sharedConfigUrl', window.wopiSettingBaseUrl);
                        formData.append('accessToken', window.accessToken);
                        return [4 /*yield*/, fetch(this.getAPIEndpoints().deleteSharedConfig, {
                                method: 'POST',
                                headers: {
                                    Authorization: "Bearer ".concat(window.accessToken),
                                },
                                body: formData,
                            })];
                    case 1:
                        response = _a.sent();
                        if (!response.ok) {
                            throw new Error("Delete failed: ".concat(response.statusText));
                        }
                        return [4 /*yield*/, this.fetchAndPopulateSharedConfigs()];
                    case 2:
                        _a.sent();
                        return [3 /*break*/, 4];
                    case 3:
                        error_5 = _a.sent();
                        SettingIframe.showErrorModal(_('Something went wrong while deleting the file. Please try refreshing the page.'));
                        console.error('Error deleting file:', error_5);
                        return [3 /*break*/, 4];
                    case 4: return [2 /*return*/];
                }
            });
        }); }, true));
        if (category === '/wordbook') {
            extraActionsDiv.appendChild(this.createButtonWithIcon('', 'edit', // Use icon key
            _('Edit'), ['button--vue-secondary', 'edit-icon'], function () { return __awaiter(_this, void 0, void 0, function () { return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.fetchWordbookFile(item.uri)];
                    case 1: return [2 /*return*/, _a.sent()];
                }
            }); }); }, true));
        }
        return extraActionsDiv;
    };
    SettingIframe.prototype.generateViewSettingUI = function (data) {
        this._viewSetting = data;
        var settingsContainer = document.getElementById('allConfigSection');
        if (!settingsContainer) {
            return;
        }
        var viewContainer = document.getElementById('view-section');
        if (viewContainer) {
            viewContainer.remove();
        }
        viewContainer = document.createElement('div');
        viewContainer.id = 'view-section';
        viewContainer.classList.add('section');
        viewContainer.appendChild(this.createHeading(_('View Settings')));
        viewContainer.appendChild(this.createParagraph(_('Adjust view settings.')));
        var divContainer = document.createElement('div');
        divContainer.id = 'view-editor';
        viewContainer.appendChild(divContainer);
        var fieldset = document.createElement('fieldset');
        fieldset.classList.add('view-settings-fieldset');
        divContainer.appendChild(fieldset);
        fieldset.appendChild(this.createLegend(_('Option')));
        for (var key in data) {
            var label = this._viewSettingLabels[key];
            if (!label) {
                continue;
            }
            if (typeof data[key] === 'boolean') {
                fieldset.appendChild(this.createViewSettingCheckbox(key, data, label));
            }
            else if (typeof data[key] === 'string') {
                fieldset.appendChild(this.createViewSettingsTextBox(key, data));
            }
        }
        viewContainer.appendChild(this.createViewSettingActions());
        settingsContainer.appendChild(viewContainer);
    };
    SettingIframe.prototype.createLegend = function (text) {
        var legend = document.createElement('legend');
        legend.textContent = text;
        return legend;
    };
    SettingIframe.prototype.createViewSettingsTextBox = function (key, data) {
        var text = data[key];
        var result = document.createElement('div');
        if (key === 'zoteroAPIKey') {
            result = this.createZoteroConfig(text, data);
        }
        return result;
    };
    SettingIframe.prototype.createViewSettingCheckbox = function (key, data, label) {
        var _this = this;
        var isChecked = data[key];
        var isDisabled = false;
        var warningText = null;
        if (key === 'accessibilityState') {
            isDisabled = !window.enableAccessibility;
            if (isDisabled) {
                warningText = _('(Warning: Server accessibility must be enabled to toggle)');
            }
        }
        // Replaced direct checkbox input creation with the new helper
        return this.createCheckbox(key, isChecked && !isDisabled, label, function (inputCheckbox, checkboxWrapper, materialIconContainer) {
            var currentChecked = !inputCheckbox.checked;
            inputCheckbox.checked = currentChecked;
            checkboxWrapper.classList.toggle('checkbox-radio-switch--checked', !currentChecked);
            materialIconContainer.innerHTML = currentChecked
                ? _this.SVG_ICONS.checkboxMarked
                : _this.SVG_ICONS.checkboxBlankOutline;
            data[key] = currentChecked;
        }, isDisabled, warningText);
    };
    SettingIframe.prototype.createViewSettingActions = function () {
        var _this = this;
        var actionsContainer = document.createElement('div');
        actionsContainer.classList.add('xcu-editor-actions');
        var resetButton = this.createButtonWithIcon('xcu-reset-button', 'reset', // Use icon key
        _('Reset to default View settings'), ['button--vue-secondary', 'xcu-reset-icon'], function (button) { return __awaiter(_this, void 0, void 0, function () {
            var confirmed, defaultViewSetting;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        confirmed = window.confirm(_('Are you sure you want to reset View Settings?'));
                        if (!confirmed) {
                            return [2 /*return*/];
                        }
                        button.disabled = true;
                        defaultViewSetting = {
                            accessibilityState: false,
                            zoteroAPIKey: '',
                        };
                        return [4 /*yield*/, this.uploadViewSettingFile('viewsetting.json', JSON.stringify(defaultViewSetting))];
                    case 1:
                        _a.sent();
                        button.disabled = false;
                        return [2 /*return*/];
                }
            });
        }); }, true);
        actionsContainer.appendChild(resetButton);
        var saveButton = this.createButtonWithText('xcu-save-button', _('Save'), _('Save View Settings'), ['button-primary'], function (button) { return __awaiter(_this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        button.disabled = true;
                        return [4 /*yield*/, this.uploadViewSettingFile('viewsetting.json', JSON.stringify(this._viewSetting))];
                    case 1:
                        _a.sent();
                        button.disabled = false;
                        return [2 /*return*/];
                }
            });
        }); });
        actionsContainer.appendChild(saveButton);
        return actionsContainer;
    };
    SettingIframe.prototype.populateSharedConfigUI = function (data) {
        return __awaiter(this, void 0, void 0, function () {
            var browserSettingButton, xcuSettingButton, fileId, fetchContent, defaultViewSetting, fileId, browserSettingContent, settingsContainer, fileId, xcuFileContent, existingXcuSection, xcuContainer;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        browserSettingButton = document.getElementById('uploadBrowserSettingsButton');
                        if (browserSettingButton) {
                            if (data.browsersetting && data.browsersetting.length > 0) {
                                browserSettingButton.style.display = 'none';
                            }
                            else {
                                browserSettingButton.style.removeProperty('display');
                            }
                        }
                        xcuSettingButton = document.getElementById('uploadXcuButton');
                        if (xcuSettingButton) {
                            if (data.xcu && data.xcu.length > 0) {
                                xcuSettingButton.style.display = 'none';
                            }
                            else {
                                xcuSettingButton.style.removeProperty('display');
                            }
                        }
                        if (!(data.kind === 'user')) return [3 /*break*/, 7];
                        if (!(data.viewsetting && data.viewsetting.length > 0)) return [3 /*break*/, 2];
                        fileId = data.viewsetting[0].uri;
                        return [4 /*yield*/, this.fetchSettingFile(fileId)];
                    case 1:
                        fetchContent = _a.sent();
                        if (fetchContent)
                            this.generateViewSettingUI(JSON.parse(fetchContent));
                        return [3 /*break*/, 3];
                    case 2:
                        defaultViewSetting = {
                            accessibilityState: false,
                            zoteroAPIKey: '',
                        };
                        this.generateViewSettingUI(defaultViewSetting);
                        _a.label = 3;
                    case 3:
                        if (!(data.browsersetting && data.browsersetting.length > 0)) return [3 /*break*/, 5];
                        fileId = data.browsersetting[0].uri;
                        return [4 /*yield*/, this.fetchSettingFile(fileId)];
                    case 4:
                        browserSettingContent = _a.sent();
                        this.browserSettingOptions = browserSettingContent
                            ? this.mergeWithDefault(defaultBrowserSetting, JSON.parse(browserSettingContent))
                            : defaultBrowserSetting;
                        return [3 /*break*/, 6];
                    case 5:
                        this.browserSettingOptions = defaultBrowserSetting;
                        _a.label = 6;
                    case 6:
                        this.createBrowserSettingForm(document.getElementById('allConfigSection'));
                        _a.label = 7;
                    case 7:
                        settingsContainer = document.getElementById('allConfigSection');
                        if (!settingsContainer)
                            return [2 /*return*/];
                        if (!(data.xcu && data.xcu.length > 0)) return [3 /*break*/, 9];
                        fileId = data.xcu[0].uri;
                        return [4 /*yield*/, this.fetchSettingFile(fileId)];
                    case 8:
                        xcuFileContent = _a.sent();
                        this.xcuEditor = new window.Xcu(this.getFilename(fileId, false), xcuFileContent);
                        existingXcuSection = document.getElementById('xcu-section');
                        if (existingXcuSection) {
                            existingXcuSection.remove();
                        }
                        xcuContainer = document.createElement('div');
                        xcuContainer.id = 'xcu-section';
                        xcuContainer.classList.add('section');
                        settingsContainer.appendChild(this.xcuEditor.createXcuEditorUI(xcuContainer));
                        return [3 /*break*/, 11];
                    case 9:
                        // If user doesn't have any xcu file, we generate with default settings...
                        this.xcuEditor = new window.Xcu('documentView.xcu', null);
                        this.xcuEditor.generateXcuAndUpload();
                        return [4 /*yield*/, this.fetchAndPopulateSharedConfigs()];
                    case 10: return [2 /*return*/, _a.sent()];
                    case 11:
                        if (data.autotext)
                            this.populateList('autotextList', data.autotext, '/autotext');
                        if (data.wordbook)
                            this.populateList('wordbookList', data.wordbook, '/wordbook');
                        if (data.xcu)
                            this.populateList('XcuList', data.xcu, '/xcu');
                        return [2 /*return*/];
                }
            });
        });
    };
    SettingIframe.prototype.mergeWithDefault = function (defaults, overrides) {
        var result = {};
        for (var key in defaults) {
            var value = defaults[key];
            var override = overrides === null || overrides === void 0 ? void 0 : overrides[key];
            if (typeof value === 'boolean' ||
                (typeof value === 'object' &&
                    value !== null &&
                    'customType' in value)) {
                // Use override directly for booleans or objects with customType (set value)
                result[key] =
                    typeof override === 'boolean'
                        ? typeof value === 'object'
                            ? __assign(__assign({}, value), { value: override }) : override
                        : value;
            }
            else if (typeof value === 'object' && value !== null) {
                result[key] = this.mergeWithDefault(value, override);
            }
            else {
                result[key] = override !== undefined ? override : value;
            }
        }
        return result;
    };
    SettingIframe.prototype.createZoteroConfig = function (APIKey, data) {
        if (APIKey === void 0) { APIKey = ''; }
        var zoteroContainer = document.createElement('div');
        zoteroContainer.id = 'zoterocontainer';
        zoteroContainer.classList.add('section');
        zoteroContainer.appendChild(this.createHeading('Zotero'));
        var zotero = this.createTextInput('zotero', _('Enter Zotero API Key'), APIKey, function (input) {
            data['zoteroAPIKey'] = input.value;
        });
        zoteroContainer.appendChild(zotero);
        return zoteroContainer;
    };
    SettingIframe.prototype.getConfigType = function () {
        return this.isAdmin() ? 'systemconfig' : 'userconfig';
    };
    SettingIframe.prototype.isAdmin = function () {
        return window.iframeType === 'admin';
    };
    SettingIframe.showErrorModal = function (message) {
        var modal = document.createElement('div');
        modal.className = 'modal';
        var modalContent = document.createElement('div');
        modalContent.className = 'modal-content';
        var header = document.createElement('h2');
        header.textContent = _('Error');
        header.style.textAlign = 'center';
        modalContent.appendChild(header);
        var messageEl = document.createElement('p');
        messageEl.textContent = message;
        modalContent.appendChild(messageEl);
        var buttonContainer = document.createElement('div');
        buttonContainer.className = 'modal-button-container';
        var okButton = document.createElement('button');
        okButton.textContent = _('OK');
        okButton.classList.add('button', 'button--vue-secondary');
        okButton.addEventListener('click', function () {
            document.body.removeChild(modal);
        });
        buttonContainer.appendChild(okButton);
        modalContent.appendChild(buttonContainer);
        modal.appendChild(modalContent);
        document.body.appendChild(modal);
    };
    SettingIframe.prototype.settingConfigBasePath = function () {
        return '/settings/' + this.getConfigType();
    };
    SettingIframe.prototype.getFilename = function (uri, removeExtension) {
        if (removeExtension === void 0) { removeExtension = true; }
        var url = new URL(uri, window.location.origin);
        var filename = url.searchParams.get('file_name');
        if (!filename) {
            // Remove query parameters from url
            uri = uri.split('?')[0];
            filename = uri.substring(uri.lastIndexOf('/') + 1);
        }
        if (removeExtension) {
            filename = filename.replace(/\.[^.]+$/, '');
        }
        return filename;
    };
    SettingIframe.prototype.toggleGridOptionsVisibility = function (checkbox) {
        var gridFieldset = checkbox.closest('.xcu-settings-fieldset');
        var childFieldsets = gridFieldset === null || gridFieldset === void 0 ? void 0 : gridFieldset.querySelectorAll('.grid-options-fieldset');
        childFieldsets === null || childFieldsets === void 0 ? void 0 : childFieldsets.forEach(function (fieldset) {
            if (checkbox.classList.contains('checkbox-radio-switch--checked')) {
                fieldset.style.display = 'none';
            }
            else {
                fieldset.style.display = 'block';
            }
        });
    };
    return SettingIframe;
}());
document.addEventListener('DOMContentLoaded', function () {
    var adminContainer = document.getElementById('allConfigSection');
    if (adminContainer) {
        initTranslationStr();
        window.settingIframe = new SettingIframe();
        window.settingIframe.init();
        var postHeight_1 = function () {
            window.parent.postMessage({
                MessageId: 'Iframe_Height',
                SendTime: Date.now(),
                Values: {
                    ContentHeight: document.documentElement.offsetHeight + 'px',
                },
            }, '*');
        };
        var timeout_1;
        var debouncePostHeight = function () {
            clearTimeout(timeout_1);
            timeout_1 = setTimeout(postHeight_1, 100);
        };
        var mutationObserver = new MutationObserver(debouncePostHeight);
        mutationObserver.observe(document.body, {
            childList: true,
            subtree: true,
            attributes: true,
            characterData: true,
        });
    }
});
window._ = _;
window.onload = onLoaded;
