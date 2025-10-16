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
/* -*- js-indent-level: 8 -*- */
/* eslint-disable */
/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
var _ = window._;
var languageMapping = {
    '<none>': 'All Language',
    ar: 'Arabic',
    bg: 'Bulgarian',
    ca: 'Catalan',
    cs: 'Czech',
    da: 'Danish',
    de: 'German',
    el: 'Greek',
    'en-US': 'English (US)',
    'en-GB': 'English (UK)',
    eo: 'Esperanto',
    es: 'Spanish',
    eu: 'Basque',
    fi: 'Finnish',
    fr: 'French',
    gl: 'Galician',
    he: 'Hebrew',
    hr: 'Croatian',
    hu: 'Hungarian',
    id: 'Indonesian',
    is: 'Icelandic',
    it: 'Italian',
    ja: 'Japanese',
    ko: 'Korean',
    lo: 'Lao',
    nb: 'Norwegian Bokmål',
    nl: 'Dutch',
    oc: 'Occitan',
    pl: 'Polish',
    pt: 'Portuguese',
    'pt-BR': 'Portuguese (Brazil)',
    sq: 'Albanian',
    ru: 'Russian',
    sk: 'Slovak',
    sl: 'Slovenian',
    sv: 'Swedish',
    tr: 'Turkish',
    uk: 'Ukrainian',
    vi: 'Vietnamese',
    'zh-CN': 'Chinese (Simplified)',
    'zh-TW': 'Chinese (Traditional)',
};
var supportedLanguages = [
    '<none>',
    'ar',
    'bg',
    'ca',
    'cs',
    'da',
    'de',
    'el',
    'en-US',
    'en-GB',
    'eo',
    'es',
    'eu',
    'fi',
    'fr',
    'gl',
    'he',
    'hr',
    'hu',
    'id',
    'is',
    'it',
    'ja',
    'ko',
    'lo',
    'nb',
    'nl',
    'oc',
    'pl',
    'pt',
    'pt-BR',
    'sq',
    'ru',
    'sk',
    'sl',
    'sv',
    'tr',
    'uk',
    'vi',
    'zh-CN',
    'zh-TW',
];
var WordBook = /** @class */ (function () {
    function WordBook() {
        this.loadingModal = null;
        this.virtualWordList = null;
        this.options = [
            {
                value: 'positive',
                heading: _('Standard dictionary'),
                description: _('Words are ignored by the spell checker'),
            },
            {
                value: 'negative',
                heading: _('Dictionary of exceptions'),
                description: _('Exception words are underlined while checking spelling'),
            },
        ];
        this.renderWordItem = function (container, word, index) {
            container.innerHTML = '';
            container.style.removeProperty('display');
            container.classList.add('list-item__wrapper');
            var listItemDiv = document.createElement('div');
            listItemDiv.classList.add('list-item');
            var wordContainer = document.createElement('div');
            wordContainer.classList.add('list-item__anchor');
            var wordContentDiv = document.createElement('div');
            wordContentDiv.classList.add('list-item-content');
            var wordTextDiv = document.createElement('div');
            wordTextDiv.classList.add('list-item-content__main');
            var wordNameDiv = document.createElement('div');
            wordNameDiv.classList.add('list-item-content__name');
            wordNameDiv.textContent = word;
            wordTextDiv.appendChild(wordNameDiv);
            wordContentDiv.appendChild(wordTextDiv);
            wordContainer.appendChild(wordContentDiv);
            listItemDiv.appendChild(wordContainer);
            var delButton = document.createElement('button');
            delButton.type = 'button';
            delButton.classList.add('button', 'button--icon-only', 'button--vue-secondary', 'delete-icon');
            delButton.innerHTML = "\n\t\t  <span class=\"button__wrapper\">\n\t\t\t<span aria-hidden=\"true\" class=\"button__icon\">\n\t\t\t  <span aria-hidden=\"true\" role=\"img\" class=\"material-design-icon\">\n\t\t\t\t<svg fill=\"currentColor\" width=\"20\" height=\"20\" viewBox=\"0 0 24 24\">\n\t\t\t\t  <path d=\"M19,4H15.5L14.5,3H9.5L8.5,4H5V6H19M6,19\n\t\t\t\t\t\t\tA2,2 0 0,0 8,21H16\n\t\t\t\t\t\t\tA2,2 0 0,0 18,19V7H6V19Z\"></path>\n\t\t\t\t</svg>\n\t\t\t  </span>\n\t\t\t</span>\n\t\t  </span>\n\t\t";
            delButton.addEventListener('click', function () {
                window.WordBook.currWordbookFile.words.splice(index, 1);
                if (window.WordBook.virtualWordList) {
                    window.WordBook.virtualWordList.refresh(window.WordBook.currWordbookFile.words);
                }
            });
            listItemDiv.appendChild(delButton);
            container.appendChild(listItemDiv);
        };
    }
    WordBook.prototype.startLoader = function () {
        this.loadingModal = document.createElement('div');
        this.loadingModal.className = 'modal';
        var loadingContent = document.createElement('div');
        loadingContent.className = 'modal-content';
        loadingContent.textContent = _('Loading Wordbook...');
        this.loadingModal.appendChild(loadingContent);
        document.body.appendChild(this.loadingModal);
    };
    WordBook.prototype.stopLoader = function () {
        if (this.loadingModal) {
            document.body.removeChild(this.loadingModal);
            this.loadingModal = null;
        }
    };
    WordBook.prototype.createDropdownElement = function (className, textContent, parent) {
        var element = document.createElement('div');
        element.className = className;
        element.textContent = textContent;
        if (parent) {
            parent.appendChild(element);
        }
        return element;
    };
    WordBook.prototype.openWordbookEditor = function (fileName, wordbook) {
        var _this = this;
        this.currWordbookFile = wordbook;
        var modal = document.createElement('div');
        modal.className = 'modal';
        var modalContent = document.createElement('div');
        modalContent.className = 'modal-content';
        var titleEl = document.createElement('h2');
        titleEl.textContent = "".concat(fileName, " (Edit)");
        titleEl.style.textAlign = 'center';
        modalContent.appendChild(titleEl);
        var languageContainer = document.createElement('div');
        languageContainer.className = 'dic-select-container';
        var languageSelect = document.createElement('select');
        languageSelect.id = 'languageSelect';
        supportedLanguages.forEach(function (code) {
            var option = document.createElement('option');
            option.value = code;
            option.textContent = languageMapping[code] || code;
            languageSelect.appendChild(option);
        });
        languageSelect.value = wordbook.language || '<none>';
        languageContainer.appendChild(languageSelect);
        modalContent.appendChild(languageContainer);
        var dictDropdownContainer = document.createElement('div');
        dictDropdownContainer.className = 'dic-dropdown-container';
        var currentDictType = wordbook.dictType || 'positive';
        dictDropdownContainer.setAttribute('data-selected', currentDictType);
        var dictDropdownSelected = document.createElement('div');
        dictDropdownSelected.className = 'dic-dropdown-selected';
        var updateSelectedDisplay = function (value) {
            while (dictDropdownSelected.firstChild) {
                dictDropdownSelected.removeChild(dictDropdownSelected.firstChild);
            }
            var opt = _this.options.find(function (o) { return o.value === value; });
            if (opt) {
                _this.createDropdownElement('dic-dropdown-heading', opt.heading, dictDropdownSelected);
                _this.createDropdownElement('dic-dropdown-description', opt.description, dictDropdownSelected);
            }
        };
        updateSelectedDisplay(currentDictType);
        dictDropdownContainer.appendChild(dictDropdownSelected);
        var dictDropdownList = document.createElement('div');
        dictDropdownList.className = 'dic-dropdown-list';
        dictDropdownList.style.display = 'none';
        var populateDropdownList = function () {
            dictDropdownList.innerHTML = '';
            var selected = dictDropdownContainer.getAttribute('data-selected');
            _this.options.forEach(function (option) {
                if (option.value !== selected) {
                    var optionDiv = document.createElement('div');
                    optionDiv.className = 'dic-dropdown-option';
                    optionDiv.setAttribute('data-value', option.value);
                    _this.createDropdownElement('dic-dropdown-heading', option.heading, optionDiv);
                    _this.createDropdownElement('dic-dropdown-description', option.description, optionDiv);
                    optionDiv.addEventListener('click', function () {
                        dictDropdownContainer.setAttribute('data-selected', option.value);
                        updateSelectedDisplay(option.value);
                        dictDropdownList.style.display = 'none';
                        dictDropdownContainer.classList.remove('open');
                    });
                    dictDropdownList.appendChild(optionDiv);
                }
            });
        };
        populateDropdownList();
        dictDropdownContainer.appendChild(dictDropdownList);
        dictDropdownSelected.addEventListener('click', function () {
            if (dictDropdownList.style.display === 'none') {
                populateDropdownList();
                dictDropdownList.style.display = 'block';
                dictDropdownContainer.classList.add('open');
            }
            else {
                dictDropdownList.style.display = 'none';
                dictDropdownContainer.classList.remove('open');
            }
        });
        modalContent.appendChild(dictDropdownContainer);
        var inputContainer = document.createElement('div');
        inputContainer.className = 'dic-input-container';
        inputContainer.style.margin = '16px 0px';
        var newWordInput = document.createElement('input');
        newWordInput.type = 'text';
        newWordInput.placeholder = _('Type to add a word');
        newWordInput.className = 'input-field__input';
        inputContainer.appendChild(newWordInput);
        var addButton = document.createElement('button');
        addButton.textContent = _('Add');
        addButton.classList.add('button', 'button--vue-secondary', 'wordbook-add-button');
        addButton.disabled = true;
        var debounceTimer;
        newWordInput.addEventListener('input', function () {
            clearTimeout(debounceTimer);
            debounceTimer = setTimeout(function () {
                var newWord = newWordInput.value.trim();
                var wordExists = _this.currWordbookFile.words.some(function (w) { return w.toLowerCase() === newWord.toLowerCase(); });
                addButton.disabled = newWord === '' || wordExists;
            }, 300);
        });
        addButton.addEventListener('click', function () {
            var newWord = newWordInput.value.trim();
            if (newWord) {
                _this.currWordbookFile.words.push(newWord);
                if (_this.virtualWordList) {
                    _this.virtualWordList.refresh(_this.currWordbookFile.words);
                    _this.virtualWordList.scrollToBottom();
                }
                newWordInput.value = '';
                addButton.disabled = true;
            }
        });
        inputContainer.appendChild(addButton);
        modalContent.appendChild(inputContainer);
        var wordList = document.createElement('ul');
        wordList.id = 'dicWordList';
        this.virtualWordList = new VirtualWordList(wordList, this.currWordbookFile.words, this.renderWordItem.bind(this));
        modalContent.appendChild(wordList);
        var buttonContainer = document.createElement('div');
        buttonContainer.className = 'dic-button-container';
        var cancelButton = document.createElement('button');
        cancelButton.textContent = _('Cancel');
        cancelButton.classList.add('button', 'button--vue-tertiary');
        cancelButton.addEventListener('click', function () {
            document.body.removeChild(modal);
        });
        buttonContainer.appendChild(cancelButton);
        var submitButton = document.createElement('button');
        submitButton.textContent = _('Save');
        submitButton.classList.add('button', 'button-primary');
        submitButton.addEventListener('click', function () { return __awaiter(_this, void 0, void 0, function () {
            var updatedContent;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        this.currWordbookFile.language = languageSelect.value;
                        this.currWordbookFile.dictType =
                            dictDropdownContainer.getAttribute('data-selected') ||
                                'positive';
                        updatedContent = this.buildWordbookFile(this.currWordbookFile);
                        console.debug('Updated Dictionary Content:\n', updatedContent);
                        return [4 /*yield*/, window.settingIframe.uploadWordbookFile(fileName, updatedContent)];
                    case 1:
                        _a.sent();
                        document.body.removeChild(modal);
                        return [2 /*return*/];
                }
            });
        }); });
        buttonContainer.appendChild(submitButton);
        modalContent.appendChild(buttonContainer);
        modal.appendChild(modalContent);
        document.body.appendChild(modal);
    };
    WordBook.prototype.parseWordbookFileAsync = function (content) {
        return new Promise(function (resolve, reject) {
            var workerCode = "\n\t\t\t\tself.onmessage = function(e) {\n\t\t\t\t\tconst content = e.data;\n\t\t\t\t\tconst lines = content.split(/\\r?\\n/).filter(line => line.trim() !== '');\n\t\t\t\t\tconst delimiterIndex = lines.findIndex(line => line.trim() === '---');\n\t\t\t\t\tif (delimiterIndex === -1) {\n\t\t\t\t\t\tself.postMessage({ error: \"Invalid dictionary format: missing delimiter '---'\" });\n\t\t\t\t\t\treturn;\n\t\t\t\t\t}\n\t\t\t\t\tif (delimiterIndex < 3) {\n\t\t\t\t\t\tself.postMessage({ error: \"Invalid dictionary format: not enough header lines before delimiter\" });\n\t\t\t\t\t\treturn;\n\t\t\t\t\t}\n\t\t\t\t\tconst headerType = lines[0].trim();\n\t\t\t\t\tconst languageMatch = lines[1].trim().match(/^lang:\\s*(.*)$/i);\n\t\t\t\t\tconst language = languageMatch ? languageMatch[1].trim() : '';\n\t\t\t\t\tconst typeMatch = lines[2].trim().match(/^type:\\s*(.*)$/i);\n\t\t\t\t\tconst dictType = typeMatch ? typeMatch[1].trim() : '';\n\t\t\t\t\tconst words = lines.slice(delimiterIndex + 1).filter(line => line.trim() !== '');\n\t\t\t\t\tself.postMessage({ result: { headerType, language, dictType, words } });\n\t\t\t\t};\n\t\t\t";
            var blob = new Blob([workerCode], {
                type: 'application/javascript',
            });
            var worker = new Worker(URL.createObjectURL(blob));
            worker.onmessage = function (e) {
                if (e.data.error) {
                    reject(new Error(e.data.error));
                }
                else {
                    resolve(e.data.result);
                }
                worker.terminate();
            };
            worker.onerror = function (e) {
                reject(e);
                worker.terminate();
            };
            worker.postMessage(content);
        });
    };
    WordBook.prototype.wordbookValidation = function (uploadPath, file) {
        return __awaiter(this, void 0, void 0, function () {
            var content, dicWords, defaultHeaderType, defaultLanguage, defaultDictType, newDic, newContent, error_1;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        _a.trys.push([0, 3, , 4]);
                        return [4 /*yield*/, this.readFileAsText(file)];
                    case 1:
                        content = _a.sent();
                        dicWords = void 0;
                        try {
                            dicWords = this.parseWords(content);
                        }
                        catch (error) {
                            window.alert(_('Invalid dictionary format. Please check the file.'));
                            console.error('Error parsing dictionary file:', error);
                            return [2 /*return*/];
                        }
                        defaultHeaderType = 'OOoUserDict1';
                        defaultLanguage = '<none>';
                        defaultDictType = 'positive';
                        newDic = {
                            headerType: defaultHeaderType,
                            language: defaultLanguage,
                            dictType: defaultDictType,
                            words: dicWords,
                        };
                        newContent = this.buildWordbookFile(newDic);
                        return [4 /*yield*/, window.settingIframe.uploadWordbookFile(file.name, newContent)];
                    case 2:
                        _a.sent();
                        return [3 /*break*/, 4];
                    case 3:
                        error_1 = _a.sent();
                        window.alert(_('Something went wrong while uploading dictionary file'));
                        return [3 /*break*/, 4];
                    case 4: return [2 /*return*/];
                }
            });
        });
    };
    WordBook.prototype.buildWordbookFile = function (wordbook) {
        var header = [
            wordbook.headerType.trim(),
            "lang: ".concat(wordbook.language),
            "type: ".concat(wordbook.dictType),
        ];
        return __spreadArray(__spreadArray(__spreadArray([], header, true), ['---'], false), wordbook.words, true).join('\n');
    };
    WordBook.prototype.readFileAsText = function (file) {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/, new Promise(function (resolve, reject) {
                        var reader = new FileReader();
                        reader.onload = function () { return resolve(reader.result); };
                        reader.onerror = function () { return reject(reader.error); };
                        reader.readAsText(file);
                    })];
            });
        });
    };
    WordBook.prototype.parseWords = function (content) {
        var lines = content
            .split(/\r?\n/)
            .map(function (line) { return line.trim(); })
            .filter(function (line) { return line !== ''; });
        var delimiterIndex = lines.findIndex(function (line) { return line === '---'; });
        if (delimiterIndex !== -1) {
            return lines.slice(delimiterIndex + 1);
        }
        return lines;
    };
    return WordBook;
}());
var VirtualWordList = /** @class */ (function () {
    function VirtualWordList(containerEl, words, renderItem) {
        var _this = this;
        this.container = containerEl;
        this.words = words;
        this.renderItem = renderItem;
        this.itemHeight = 50;
        this.container.style.overflowY = 'auto';
        this.contentWrapper = document.createElement('div');
        this.contentWrapper.style.position = 'relative';
        this.visibleCount = 7;
        this.contentWrapper.style.height = "".concat(this.visibleCount * this.itemHeight, "px");
        this.container.appendChild(this.contentWrapper);
        this.viewport = document.createElement('div');
        this.viewport.style.position = 'absolute';
        this.viewport.style.top = '0';
        this.viewport.style.left = '0';
        this.viewport.style.right = '0';
        this.contentWrapper.appendChild(this.viewport);
        this.pool = [];
        for (var i = 0; i < this.visibleCount; i++) {
            var li = document.createElement('li');
            li.style.display = 'none';
            li.style.height = this.itemHeight + 'px';
            this.pool.push(li);
            this.viewport.appendChild(li);
        }
        this.container.addEventListener('scroll', function () { return _this.onScroll(); });
        this.onScroll();
    }
    VirtualWordList.prototype.onScroll = function () {
        var scrollTop = this.container.scrollTop;
        var firstIndex = Math.floor(scrollTop / this.itemHeight);
        var maxFirstIndex = Math.max(0, this.words.length - this.pool.length);
        if (firstIndex > maxFirstIndex) {
            firstIndex = maxFirstIndex;
        }
        this.viewport.style.transform = "translateY(".concat(firstIndex * this.itemHeight, "px)");
        for (var i = 0; i < this.pool.length; i++) {
            var wordIndex = firstIndex + i;
            if (wordIndex < this.words.length) {
                this.renderItem(this.pool[i], this.words[wordIndex], wordIndex);
            }
            else {
                this.pool[i].innerHTML = '';
            }
        }
    };
    VirtualWordList.prototype.refresh = function (newWords) {
        if (newWords) {
            this.words = newWords;
        }
        this.onScroll();
    };
    VirtualWordList.prototype.scrollToBottom = function () {
        this.container.scrollTop = this.visibleCount * this.itemHeight;
    };
    return VirtualWordList;
}());
window.WordBook = new WordBook();
