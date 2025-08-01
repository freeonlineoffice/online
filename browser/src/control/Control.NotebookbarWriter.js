/* -*- js-indent-level: 8 -*- */
/*
 * L.Control.NotebookbarWriter
 */

/* global _ _UNO app */

var fileTabName = 'File';
var homeTabName = 'Home';
var insertTabName = 'Insert';
var layoutTabName = 'Layout';
var referencesTabName = 'References';
var reviewTabName = 'Review';
var formatTabName = 'Format';
var formTabName = 'Form';
var tableTabName = 'Table';
var shapeTabName = 'Shape';
var pictureTabName = 'Picture';
var viewTabName = 'View';
var helpTabName = 'Help';
var formulaTabName = 'Formula';

L.Control.NotebookbarWriter = L.Control.Notebookbar.extend({

	getTabs: function() {
		return [
			{
				'text': _('File'),
				'id': fileTabName + '-tab-label',
				'name': fileTabName,
				'accessibility': { focusBack: true, combination: 'F', de: 'D' }
			},
			{
				'text': _('Home'),
				'id': this.HOME_TAB_ID,
				'name': homeTabName,
				'context': 'default|Text|DrawText',
				'accessibility': { focusBack: true, combination: 'H', de: 'R' }
			},
			{
				'text': _('Insert'),
				'id': insertTabName + '-tab-label',
				'name': insertTabName,
				'accessibility': { focusBack: true, combination: 'N', de: 'I' }
			},
			{
				'text': _('Layout'),
				'id': layoutTabName + '-tab-label',
				'name': layoutTabName,
				'accessibility': { focusBack: true, combination: 'P', de: 'S' }
			},
			{
				'text': _('References'),
				'id': referencesTabName + '-tab-label',
				'name': referencesTabName,
				'accessibility': { focusBack: true, combination: 'S', de: 'C' }
			},
			{
				'text': _('Review'),
				'id': reviewTabName + '-tab-label',
				'name': reviewTabName,
				'accessibility': { focusBack: true, combination: 'R', de: 'P' }
			},
			{
				'text': _('Format'),
				'id': formatTabName + '-tab-label',
				'name': formatTabName,
				'accessibility': { focusBack: true, combination: 'O' }
			},
			{
				'text': _('Form'),
				'id': formTabName + '-tab-label',
				'name': formTabName,
				'accessibility': { focusBack: true, combination: 'M' }
			},
			{
				'text': _('Table'),
				'id': tableTabName + '-tab-label',
				'name': tableTabName,
				'context': 'Table',
				'accessibility': { focusBack: true, combination: 'T' }
			},
			{
				'text': _('Shape'),
				'id': shapeTabName + '-tab-label',
				'name': shapeTabName,
				'context': 'Draw|DrawLine|3DObject|MultiObject|DrawFontwork',
				'accessibility': { focusBack: true, combination: 'JI', de: 'JI' }
			},
			{
				'text': _('Picture'),
				'id': pictureTabName + '-tab-label',
				'name': pictureTabName,
				'context': 'Graphic',
				'accessibility': { focusBack: true, combination: 'PI', de: 'PI' }
			},
			{
				'text': _('View'),
				'id': viewTabName + '-tab-label',
				'name': viewTabName,
				'accessibility': { focusBack: true, combination: 'W', de: 'F' }
			},
			{
				'text': _('Help'),
				'id': helpTabName + '-tab-label',
				'name': helpTabName,
				'accessibility': { focusBack: true, combination: 'Y', de: 'E' }
			},
			{
				'text': _('Formula'),
				'id': formulaTabName + '-tab-label',
				'name': formulaTabName,
				'context': 'Math',
				'accessibility': { focusBack: true, combination: 'V', de: 'Y' }
			}
		];
	},

	getTabsJSON: function () {
		return [
			this.getFileTab(),
			this.getHomeTab(),
			this.getInsertTab(),
			this.getLayoutTab(),
			this.getReferencesTab(),
			this.getReviewTab(),
			this.getFormatTab(),
			this.getFormTab(),
			this.getTableTab(),
			this.getShapeTab(),
			this.getPictureTab(),
			this.getViewTab(),
			this.getHelpTab(),
			this.getFormulaTab()
		]
	},

	getFullJSON: function (selectedId) {
		return this.getNotebookbar(this.getTabsJSON(), selectedId);
	},

	getFileTab: function() {
		var hasRevisionHistory = L.Params.revHistoryEnabled;
		var hasPrint = !this.map['wopi'].HidePrintOption;
		var hasRepair = !this.map['wopi'].HideRepairOption;
		var hasSaveAs = !this.map['wopi'].UserCanNotWriteRelative;
		var hasShare = this.map['wopi'].EnableShare;
		var hideDownload = this.map['wopi'].HideExportOption;
		var hasGroupedSaveAs = window.prefs.get('saveAsMode') === 'group';
		var hasRunMacro = window.enableMacrosExecution;
		var hasSave = !this.map['wopi'].HideSaveOption;
		var content = [];

		if (hasSave) {
			content.push({
				'type': 'toolbox',
				'children': [
					{
						'id': 'file-save',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:Save'),
						'command': '.uno:Save',
						'accessibility': { focusBack: true,	combination: 'SV', de: null }
					}
				]
			});
		}

		if (hasSaveAs) {
			if (hasGroupedSaveAs) {
				content.push({
					'id': 'saveas:SaveAsMenu',
					'command': 'saveas',
					'type': 'exportmenubutton',
					'text': _('Save As'),
					'accessibility': { focusBack: true,	combination: 'SA' }
				});
			} else {
				content.push({
					'id': 'file-saveas',
					'type': 'bigtoolitem',
					'text': _UNO('.uno:SaveAs', 'text'),
					'command': '.uno:SaveAs',
					'accessibility': { focusBack: true,	combination: 'SA' }
				});
			}
		}

		if (hasSaveAs) {
			content.push({
				'id': 'exportas:ExportAsMenu',
				'command': 'exportas',
				'class': 'unoexportas',
				'type': 'exportmenubutton',
				'text': _('Export As'),
				'accessibility': { focusBack: true,	combination: 'EA' }
			});
		}

		if (hasShare && hasRevisionHistory) {
			content.push(
				{
					'id': 'file-exportas-break',
					'type': 'separator',
					'orientation': 'vertical'
				},
				{
					'type': 'container',
					'children': [
						{
							'id': 'ShareAs',
							'class': 'unoShareAs',
							'type': 'customtoolitem',
							'text': _('Share'),
							'command': 'shareas',
							'inlineLabel': true,
							'accessibility': { focusBack: true, combination: 'SH' }
						}, {
							'id': 'Rev-History',
							'class': 'unoRev-History',
							'type': 'customtoolitem',
							'text': _('See history'),
							'command': 'rev-history',
							'inlineLabel': true,
							'accessibility': { focusBack: true, combination: 'RH' }
						}					],
					'vertical': true
				}, {
					'id': 'file-revhistory-break',
					'type': 'separator',
					'orientation': 'vertical'
				}
			);
		} else if (hasShare) {
			content.push(
				{
					'id': 'file-exportas-break',
					'type': 'separator',
					'orientation': 'vertical'
				}, {
				'type': 'container',
				'children': [
					{
						'id': 'ShareAs',
						'class': 'unoShareAs',
						'type': 'bigcustomtoolitem',
						'text': _('Share'),
						'command': 'shareas',
						'accessibility': { focusBack: true, combination: 'SH' }
					}
				]
				}, {
					'id': 'file-shareas-break',
					'type': 'separator',
					'orientation': 'vertical'
				}
			);
		} else if (hasRevisionHistory) {
			content.push(
				{
					'id': 'file-exportas-break',
					'type': 'separator',
					'orientation': 'vertical'
				}, {
					'type': 'container',
					'children': [
						{
							'id': 'Rev-History',
							'class': 'unoRev-History',
							'type': 'bigcustomtoolitem',
							'text': _('See history'),
							'command': 'rev-history',
							'accessibility': { focusBack: true, combination: 'RH' }
						}
					]
				}, {
						'id': 'file-revhistory-break',
						'type': 'separator',
						'orientation': 'vertical'
				}
			);
		}

		if (hasPrint) {
			content.push(
			{
				'id': 'print',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:Print', 'text'),
				'command': '.uno:Print',
				'accessibility': { focusBack: true,	combination: 'P', de: 'P' }
			});
		}

		if (hasRunMacro) {
			content.push(
			{
				'type': 'toolbox',
				'children': [
					{
						'id': 'runmacro',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:RunMacro', 'text'),
						'command': '.uno:RunMacro'
					}
				]
			});
		}

		if (!hideDownload) {
			content.push({
				'id': 'downloadas:DownloadAsMenu',
				'command': 'downloadas',
				'class': 'unodownloadas',
				'type': 'exportmenubutton',
				'text': !window.ThisIsAMobileApp ? _('Download') : _('Save As'),
				'accessibility': { focusBack: true,	combination: 'A', de: 'M' }
			});
		}

		content.push( { 'id': 'file-downloadas-break', 'type': 'separator', 'orientation': 'vertical' } );

		if (hasRepair) {
			content.push({
				'type': 'container',
				'children': [
					{
						'id': 'repair',
						'class': 'unorepair',
						'type': 'bigcustomtoolitem',
						'text': _('Repair'),
						'accessibility': { focusBack: true,	combination: 'RF', de: null }
					}
				],
				'vertical': 'true'
			});
		}

		content.push(
			{
				'type': 'container',
				'children': [
					{
						'id': 'properties',
						'type': 'bigtoolitem',
						'text': _('Properties'),
						'command': '.uno:SetDocumentProperties',
						'accessibility': { focusBack: true,	combination: 'FP', de: 'I' }
					}
				]
		});
		if (window.documentSigningEnabled) {
			content.push({
				'type': 'container',
				'children': [
					{
						'id': 'signature',
						'type': 'bigtoolitem',
						'text': _('Signature'),
						'command': '.uno:Signature',
						'accessibility': { focusBack: true, combination: 'SN' }
					}
				]
			});
		}

		if (this._map['wopi']._supportsRename() && this._map['wopi'].UserCanRename) {
			content.push({
				'type': 'container',
				'children': [
					{
						'id': 'renamedocument',
						'class': 'unoRenameDocument',
						'type': 'bigcustomtoolitem',
						'text': _('Rename'),
						'accessibility': { focusBack: true,	combination: 'RN' },
					}
				]
			});
		}

		if (window.wasmEnabled) {
			content.push({
				'type': 'container',
				'children': [
					{ type: 'separator', id: 'file-renamedocument-break', orientation: 'vertical' },
					{
						'id': 'togglewasm',
						'class': 'togglewasm',
						'type': 'bigcustomtoolitem',
						'text': _(window.ThisIsTheEmscriptenApp ? _('Go Online') : _('Go Offline')),
						'accessibility': { focusBack: true, combination: 'RN' }
					}
				]
			});
		}

		return this.getTabPage(fileTabName, content);
	},

	getHelpTab: function() {
		var hasLatestUpdates = window.enableWelcomeMessage;
		var hasFeedback = this.map.feedback;
		var hasAccessibilitySupport = window.enableAccessibility;
		var hasAccessibilityCheck = this.map.getDocType() === 'text';
		var hasAbout = L.DomUtil.get('about-dialog') !== null;
		var hasServerAudit = !!this.map.serverAuditDialog;

		var content = [
			{
				'type': 'container',
				'id': helpTabName + '-container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'forum',
								'type': 'bigtoolitem',
								'text': _('Forum'),
								'command': '.uno:ForumHelp',
								'accessibility': { focusBack: true, combination: 'C', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'online-help',
								'type': 'bigtoolitem',
								'text': _('Online Help'),
								'command': '.uno:OnlineHelp',
								'accessibility': { focusBack: false, combination: 'H', de: null }
							}
						]
					},
					{ type: 'separator', id: 'help-onlinehelp-break', orientation: 'vertical' },
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'keyboard-shortcuts',
								'type': 'bigtoolitem',
								'text': _('Keyboard shortcuts'),
								'command': '.uno:KeyboardShortcuts',
								'accessibility': { focusBack: false, combination: 'S', de: null }
							}
						]
					},
					{ type: 'separator', id: 'help-keyboardshortcuts-break', orientation: 'vertical' },
					hasAccessibilitySupport ?
						{
							'id':'togglea11ystate',
							'type': 'bigcustomtoolitem',
							'text': _('Screen Reading'),
							'accessibility': { focusBack: true,	combination: 'SR', de: null }
						} : {},
					hasAccessibilityCheck ?
						{
							'id': 'accessibility-check',
							'class': 'unoAccessibilityCheck',
							'type': 'bigtoolitem',
							'text': _UNO('.uno:AccessibilityCheck', 'text'),
							'command': '.uno:SidebarDeck.A11yCheckDeck',
							'accessibility': { focusBack: false, combination: 'A', de: null }
						} : {},
					hasAccessibilitySupport || hasAccessibilityCheck ?
						{
							'id': 'help-accessibility-break',
							'type': 'separator',
							'orientation': 'vertical'
						} : {},
					hasServerAudit ?
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'server-audit',
								'type': 'bigcustomtoolitem',
								'text': _('Server audit'),
								'command': 'serveraudit',
								'accessibility': { focusBack: false, combination: 'SA', de: null }
							},
							{
								'id': 'help-serveraudit-break',
								'type': 'separator',
								'orientation': 'vertical'
							}
						]
					} : {},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'report-an-issue',
								'type': 'bigtoolitem',
								'text': _('Report an issue'),
								'command': '.uno:ReportIssue',
								'accessibility': { focusBack: true, combination: 'K', de: null }
							},
							{ 'type': 'separator', 'id': 'help-reportissue-break', 'orientation': 'vertical' },
						]
					},
					hasLatestUpdates ?
						{
							'type': 'toolbox',
							'children': [
								{
									'id': 'latestupdates',
									'type': 'bigtoolitem',
									'text': _('Latest Updates'),
									'command': '.uno:LatestUpdates',
									'accessibility': { focusBack: true,	combination: 'LU', de: null }

								}
							]
						} : {},
					hasFeedback ?
						{
							'type': 'toolbox',
							'children': [
								{
									'id': 'feedback',
									'type': 'bigtoolitem',
									'text': _('Send Feedback'),
									'command': '.uno:Feedback',
									'accessibility': { focusBack: true,	combination: 'SF', de: null }
								}
							]
						} : {},
					hasAbout ?
						{
							'type': 'toolbox',
							'children': [
								{
									'id': 'about',
									'type': 'bigtoolitem',
									'text': _('About'),
									'command': '.uno:About',
									'accessibility': { focusBack: false, combination: 'W', de: null }
								}
							]
						} : {}
				]
			}
		];

		return this.getTabPage(helpTabName, content);
	},

	getHomeTab: function() {
		var content = [
			{
				'id': 'home-undo-redo',
				'type': 'container',
				'children': [
					{
						'id': 'home-undo',
						'type': 'toolitem',
						'text': _UNO('.uno:Undo'),
						'command': '.uno:Undo',
						'accessibility': { focusBack: true,	combination: 'ZZ',	de: 'ZZ' }
					},
					{
						'id': 'home-redo',
						'type': 'toolitem',
						'text': _UNO('.uno:Redo'),
						'command': '.uno:Redo',
						'accessibility': { focusBack: true,	combination: 'O',	de: 'W' }
					},
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'home-undoredo-break', orientation: 'vertical' },
			{
				'id': 'home-paste:PasteMenu',
				'type': 'menubutton',
				'text': _UNO('.uno:Paste'),
				'command': '.uno:Paste',
				'accessibility': { focusBack: false,	combination: 'V',	de: null }
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'home-cut',
								'type': 'customtoolitem',
								'text': _UNO('.uno:Cut'),
								'command': '.uno:Cut',
								'accessibility': { focusBack: true, 	combination: 'X',	de: 'X' }
							},
							{
								'id': 'home-brush',
								'type': 'toolitem',
								'text': _UNO('.uno:FormatPaintbrush'),
								'command': '.uno:FormatPaintbrush',
								'accessibility': { focusBack: true,	combination: 'FP',	de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'home-copy',
								'type': 'customtoolitem',
								'text': _UNO('.uno:Copy'),
								'command': '.uno:Copy',
								'accessibility': { focusBack: true, 	combination: 'C',	de: 'C' }
							},
							{
								'id': 'home-reset-attributes',
								'type': 'toolitem',
								'text': _UNO('.uno:ResetAttributes'),
								'command': '.uno:ResetAttributes',
								'accessibility': { focusBack: true, 	combination: 'E',	de: 'Q' }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'home-resertattributes-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'container',
						'children': [
							{
								'id': 'fontnamecombobox',
								'type': 'combobox',
								'text': 'Carlito',
								'entries': [
									'Carlito'
								],
								'selectedCount': '1',
								'selectedEntries': [
									'0'
								],
								'command': '.uno:CharFontName',
								'accessibility': { focusBack: false,	combination: 'FF',	de: null }
							},
							{
								'id': 'fontsizecombobox',
								'type': 'combobox',
								'text': '12 pt',
								'entries': [
									'12 pt'
								],
								'selectedCount': '1',
								'selectedEntries': [
									'0'
								],
								'command': '.uno:FontHeight',
								'accessibility': { focusBack: false,	combination: 'FS',	de: null }
							},
							{
								'id': 'home-grow',
								'type': 'toolitem',
								'text': _UNO('.uno:Grow'),
								'command': '.uno:Grow',
								'accessibility': { focusBack: true, 	combination: 'FG',	de: 'SV' }
							},
							{
								'id': 'home-shrink',
								'type': 'toolitem',
								'text': _UNO('.uno:Shrink'),
								'command': '.uno:Shrink',
								'accessibility': { focusBack: true, 	combination: 'FK',	de: 'J' }
							}
						],
						'vertical': 'false'
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'home-bold',
										'type': 'toolitem',
										'text': _UNO('.uno:Bold'),
										'command': '.uno:Bold',
										'accessibility': { focusBack: true, 	combination: '1',	de: '1' }
									},
									{
										'id': 'home-italic',
										'type': 'toolitem',
										'text': _UNO('.uno:Italic'),
										'command': '.uno:Italic',
										'accessibility': { focusBack: true, 	combination: '2',	de: '2' }
									},
									{
										'id': 'home-underline',
										'type': 'toolitem',
										'text': _UNO('.uno:Underline'),
										'command': '.uno:Underline',
										'accessibility': { focusBack: true, 	combination: '3',	de: '3' }
									},
									{
										'id': 'home-strikeout',
										'type': 'toolitem',
										'text': _UNO('.uno:Strikeout'),
										'command': '.uno:Strikeout',
										'accessibility': { focusBack: true, 	combination: '4',	de: '4' }
									},
									{
										'id': 'home-subscript',
										'type': 'toolitem',
										'text': _UNO('.uno:SubScript'),
										'command': '.uno:SubScript',
										'accessibility': { focusBack: true, 	combination: '5',	de: '5' }
									},
									{
										'id': 'home-superscript',
										'type': 'toolitem',
										'text': _UNO('.uno:SuperScript'),
										'command': '.uno:SuperScript',
										'accessibility': { focusBack: true, 	combination: '6',	de: '6' }
									},
									{
										'id': 'home-spacing:CharSpacingMenu',
										'type': 'menubutton',
										'noLabel': true,
										'text': _UNO('.uno:Spacing'),
										'command': '.uno:CharSpacing',
										'accessibility': { focusBack: false,	combination: 'FT',	de: null }
									},
									{
										'id': 'home-back-color:ColorPickerMenu',
										'class': 'unospan-CharBackColor',
										'type': 'toolitem',
										'noLabel': true,
										'text': _UNO('.uno:CharBackColor', 'text'),
										'command': '.uno:CharBackColor',
										'accessibility': { focusBack: true,	combination: 'HC',	de:	null }
									},
									{
										'id': 'home-color:ColorPickerMenu',
										'class': 'unospan-FontColor',
										'type': 'toolitem',
										'noLabel': true,
										'text': _UNO('.uno:Color'),
										'command': '.uno:Color',
										'accessibility': { focusBack: true,	combination: 'FC',	de: null }
									}
								]
							}
						],
						'vertical': 'false'
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'home-fontcombobox-break', orientation: 'vertical' },
			{
				'id': 'home-insert-annotation',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:InsertAnnotation'),
				'command': '.uno:InsertAnnotation',
				'accessibility': { focusBack: false, combination: 'ZC', de: 'ZC' }
			},
			{ type: 'separator', id: 'home-insertannotation-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'home-default-bullet',
										'type': 'toolitem',
										'text': _UNO('.uno:DefaultBullet'),
										'command': '.uno:DefaultBullet',
										'accessibility': { focusBack: true, 	combination: 'U',	de: 'AA' }
									},
									{
										'id': 'home-default-numbering',
										'type': 'toolitem',
										'text': _UNO('.uno:DefaultNumbering'),
										'command': '.uno:DefaultNumbering',
										'accessibility': { focusBack: true, 	combination: 'N',	de: 'GN' }
									},
									{
										'id': 'home-increment-indent',
										'type': 'toolitem',
										'text': _UNO('.uno:IncrementIndent'),
										'command': '.uno:IncrementIndent',
										'accessibility': { focusBack: true, 	combination: 'AI',	de: 'ÖI' }
									},
									{
										'id': 'home-decrement-indent',
										'type': 'toolitem',
										'text': _UNO('.uno:DecrementIndent'),
										'command': '.uno:DecrementIndent',
										'accessibility': { focusBack: true, 	combination: 'AO',	de: 'PI' }
									},
									{
										'id': 'home-control-codes',
										'type': 'toolitem',
										'text': _UNO('.uno:ControlCodes', 'text'),
										'command': '.uno:ControlCodes',
										'accessibility': { focusBack: true, 	combination: 'FM',	de: 'FM' }
									},
									{
										'id': 'home-para-left-to-right',
										'type': 'toolitem',
										'text': _UNO('.uno:ParaLeftToRight'),
										'command': '.uno:ParaLeftToRight',
										'accessibility': { focusBack: true, 	combination: 'TL',	de: 'EB' }
									},
									{
										'id': 'home-para-right-to-left',
										'type': 'toolitem',
										'text': _UNO('.uno:ParaRightToLeft'),
										'command': '.uno:ParaRightToLeft',
										'accessibility': { focusBack: true,	combination: 'TR', de: null }
									}
								]
							},
						],
						'vertical': 'false'
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'home-left-para',
										'type': 'toolitem',
										'text': _UNO('.uno:LeftPara'),
										'command': '.uno:LeftPara',
										'accessibility': { focusBack: true, 	combination: 'AL',	de: 'AL' }
									},
									{
										'id': 'home-center-para',
										'type': 'toolitem',
										'text': _UNO('.uno:CenterPara'),
										'command': '.uno:CenterPara',
										'accessibility': { focusBack: true, 	combination: 'AC',	de: 'RZ' }
									},
									{
										'id': 'home-right-para',
										'type': 'toolitem',
										'text': _UNO('.uno:RightPara'),
										'command': '.uno:RightPara',
										'accessibility': { focusBack: true, 	combination: 'AR',	de: 'RE' }
									},
									{
										'id': 'home-justify-para',
										'type': 'toolitem',
										'text': _UNO('.uno:JustifyPara'),
										'command': '.uno:JustifyPara',
										'accessibility': { focusBack: true, 	combination: 'AJ',	de: 'OL' }
									},
									{
										'id': 'home-line-spacing:LineSpacingMenu',
										'type': 'menubutton',
										'noLabel': true,
										'text': _UNO('.uno:LineSpacing'),
										'command': '.uno:LineSpacing',
										'accessibility': { focusBack: false,	combination: 'K',	de: null }
									},
									{
										'id': 'home-background-color:ColorPickerMenu',
										'class': 'unospan-BackgroundColor',
										'noLabel': true,
										'type': 'toolitem',
										'text': _UNO('.uno:BackgroundColor'),
										'command': '.uno:BackgroundColor',
										'accessibility': { focusBack: true,	combination: 'BC',	de: null }
									}
								]
							},
						],
						'vertical': 'false'
					}
				],
				'vertical': 'true'
			},
			{
				'id': 'stylesview',
				'type': 'iconview',
				'entries': [],
				'vertical': 'false'
			},
			{
				'id': 'stylesview-btn',
				'type': 'container',
				'children': [
					{
						'id': 'scroll-up',
						'type': 'customtoolitem',
						'text': _('Scroll up'),
						'command': 'scrollpreviewup',
						'icon': 'lc_searchprev.svg',
					},
					{
						'id': 'scroll-down',
						'type': 'customtoolitem',
						'text': _('Scroll down'),
						'command': 'scrollpreviewdown',
						'icon': 'lc_searchnext.svg',
					},
					{
						'id': 'format-style-list-dialog',
						'type': 'toolitem',
						'text': _('Style list'),
						'command': '.uno:SidebarDeck.StyleListDeck',
						'icon': 'lc_stylepreviewmore.svg',
						'accessibility': { focusBack: true, combination: 'SD', de: null }
					},
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'home-stylesview-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'home-insert-table:InsertTableMenu',
								'type': 'menubutton',
								'noLabel': true,
								'text': _UNO('.uno:InsertTable', 'text'),
								'command': '.uno:InsertTable',
								'accessibility': { focusBack: false,	combination: 'IT',	de:	null }
							},
							{
								'id': 'home-insert-graphic:InsertImageMenu',
								'type': 'menubutton',
								'noLabel': true,
								'text': _UNO('.uno:InsertGraphic'),
								'command': '.uno:InsertGraphic',
								'accessibility': { focusBack: true, 	combination: 'IG',	de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'home-insert-page-break',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertPagebreak', 'text'),
								'command': '.uno:InsertPagebreak',
								'accessibility': { focusBack: true, 	combination: 'IP',	de: null }
							},
							{
								'id': 'CharmapControl',
								'class': 'unoCharmapControl',
								'type': 'customtoolitem',
								'text': _UNO('.uno:CharmapControl'),
								'command': 'charmapcontrol',
								'accessibility': { focusBack: false,	combination: 'IS',	de:	null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'home-charmapcontrol-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
								{
									'id': 'home-search',
									'class': 'unoSearch',
									'type': 'customtoolitem',
									'text': _('Search'),
									'command': 'search',
									'accessibility': { focusBack: false,	combination: 'SS',	de: 'SS' }
								}
							]
						},
						{
							'type': 'toolbox',
							'children': [
								{
									'id': 'home-search-dialog',
									'type': 'toolitem',
									'text': _('Replace'),
									'command': '.uno:SearchDialog',
									'accessibility': { focusBack: false, 	combination: 'FD',	de: 'US' }
								}
							]
						}
					],
				'vertical': 'true'
			},
		];

		return this.getTabPage(homeTabName, content);
	},

	getFormatTab: function() {
		var content = [
			{
				'id': 'format-font-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FontDialog', 'text'),
				'command': '.uno:FontDialog',
				'accessibility': { focusBack: false, combination: 'A', de: null }
			},
			{
				'id': 'format-FormatMenu:FormatMenu',
				'type': 'menubutton',
				'text': _UNO('.uno:FormatMenu', 'text'),
				'command': '.uno:FormatMenu',
				'accessibility': { focusBack: false, combination: 'FT', de: null }
			},
			{
				'id': 'format-paragraph-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:ParagraphDialog', 'text'),
				'command': '.uno:ParagraphDialog',
				'accessibility': { focusBack: false, combination: 'B', de: null }
			},
			{
				'id': 'format-style-dialog',
				'type': 'bigtoolitem',
				'text': _('Style list'),
				'command': '.uno:SidebarDeck.StyleListDeck',
				'accessibility': { focusBack: false, combination: 'SD', de: null }
			},
			{ type: 'separator', id: 'format-styledialog-break', orientation: 'vertical' },
			{
				'id': 'format-FormatBulletsMenu:FormatBulletsMenu',
				'type': 'menubutton',
				'text': _UNO('.uno:FormatBulletsMenu', 'text'),
				'command': '.uno:FormatBulletsMenu',
				'accessibility': { focusBack: false, combination: 'FB', de: null }
			},
			{
				'id': 'format-outline-bullet',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:OutlineBullet', 'text'),
				'command': '.uno:OutlineBullet',
				'accessibility': { focusBack: false, combination: 'C', de: null }
			},
			{ type: 'separator', id: 'format-outlinebullet-break', orientation: 'vertical' },
			{
				'id': 'format-page-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:PageDialog', 'text'),
				'command': '.uno:PageDialog',
				'accessibility': { focusBack: false, combination: 'D', de: null }
			},
			{ type: 'separator', id: 'format-pagedialog-break', orientation: 'vertical' },
			{
				'id': 'format-format-columns',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FormatColumns', 'text'),
				'command': '.uno:FormatColumns',
				'accessibility': { focusBack: false, combination: 'E', de: null }
			},
			{
				'id': 'format-edit-region',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:EditRegion', 'text'),
				'command': '.uno:EditRegion',
				'accessibility': { focusBack: false, combination: 'F', de: null }
			},
			{
				'id': 'format-format-line',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FormatLine'),
				'command': '.uno:FormatLine',
				'accessibility': { focusBack: false, combination: 'G', de: null }
			},
			{
				'id': 'format-format-area',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FormatArea'),
				'command': '.uno:FormatArea'
			},
			{ type: 'separator', id: 'format-formatarea-break', orientation: 'vertical' },
			{
				'id': 'format-transform-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TransformDialog'),
				'command': '.uno:TransformDialog',
				'accessibility': { focusBack: false, combination: 'H', de: null }
			},
			{ type: 'separator', id: 'format-transformdialog-break', orientation: 'vertical' },
			{
				'id': 'format-chapter-numbering-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:ChapterNumberingDialog', 'text'),
				'command': '.uno:ChapterNumberingDialog',
				'accessibility': { focusBack: false, combination: 'I', de: null }
			},
			{ type: 'separator', id: 'format-chapternumberingdialog-break', orientation: 'vertical' },
			{
				'id': 'format-name-description',
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'format-name-group',
								'type': 'toolitem',
								'text': _UNO('.uno:NameGroup', 'text'),
								'command': '.uno:NameGroup',
								'accessibility': { focusBack: false, combination: 'NG', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'format-object-title-description',
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectTitleDescription', 'text'),
								'command': '.uno:ObjectTitleDescription',
								'accessibility': { focusBack: false, combination: 'DS', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'format-objecttitledescription-break', orientation: 'vertical' },
			{
				'id': 'format-theme-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:ThemeDialog'),
				'command': '.uno:ThemeDialog',
				'accessibility': { focusBack: false, combination: 'J', de: null }
			}
		];

		return this.getTabPage(formatTabName, content);
	},

	getInsertTab: function() {
		var isODF = app.LOUtil.isFileODF(this.map);
		var content = [
			{
				'id': 'insert-insert-page-break',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:InsertPagebreak', 'text'),
				'command': '.uno:InsertPagebreak',
				'accessibility': { focusBack: true,	combination: 'B',	de:	'SU' }
			},
			{ type: 'separator', id: 'insert-insertpagebreak-break', orientation: 'vertical' },
			{
				'id': 'insert-insert-table:InsertTableMenu',
				'type': 'menubutton',
				'text': _('Table'),
				'command': '.uno:InsertTable',
				'accessibility': { focusBack: false,	combination: 'IT',	de: null }
			},
			{ type: 'separator', id: 'insert-inserttable-break', orientation: 'vertical' },
			{
				'id': 'insert-insert-graphic:InsertImageMenu',
				'type': 'menubutton',
				'text': _UNO('.uno:InsertGraphic'),
				'command': '.uno:InsertGraphic',
				'accessibility': { focusBack: true,	combination: 'P',	de:	'BI' }
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-shapes:InsertShapesMenu',
								'type': 'menubutton',
								'noLabel': true,
								'text': _('Shapes'),
								'command': '.uno:BasicShapes',
								'accessibility': { focusBack: false,	combination: 'IH',	de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-object-chart',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertObjectChart'),
								'command': '.uno:InsertObjectChart',
								'accessibility': { focusBack: false,	combination: 'C',	de:	null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'insert-insertobjectchart-break', orientation: 'vertical' },
			(this.map['wopi'].EnableRemoteLinkPicker) ? {
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-hyperlink-dialog',
								'class': 'unoHyperlinkDialog',
								'type': 'customtoolitem',
								'text': _UNO('.uno:HyperlinkDialog'),
								'command': 'hyperlinkdialog',
								'accessibility': { focusBack: false,	combination: 'ZL',	de:	'8' }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-remote-link',
								'class': 'unoremotelink',
								'type': 'customtoolitem',
								'text': _('Smart Picker'),
								'command': 'remotelink',
								'accessibility': { focusBack: true, combination: 'RL', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			} : {
				'id': 'insert-hyperlink-dialog',
				'class': 'unoHyperlinkDialog',
				'type': 'bigcustomtoolitem',
				'text': _UNO('.uno:HyperlinkDialog'),
				'command': 'hyperlinkdialog',
				'accessibility': { focusBack: false,	combination: 'ZL',	de:	'8' }
			},
			{ type: 'separator', id: 'insert-hyperlinkdialog-break', orientation: 'vertical' },
			(this.map['wopi'].EnableRemoteAIContent) ? {
				'id': 'insert-insert-remote-ai-content',
				'class': 'unoremoteaicontent',
				'type': 'bigcustomtoolitem',
				'text': _('Assistant'),
				'command': 'remoteaicontent',
				'accessibility': { focusBack: true, combination: 'RL', de: null }
			} : {},
			(this.map['wopi'].EnableRemoteAIContent) ? {
				'type': 'bigcustomtoolitem',
				'id': 'insert-remoteaicontent-break',
				'orientation': 'vertical'
			} : {},
			{
				'id': 'insert-insert-annotation',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:InsertAnnotation', 'text'),
				'command': '.uno:InsertAnnotation',
				'accessibility': { focusBack: false, combination: 'ZC', de: 'ZC' }
			},
			{ type: 'separator', id: 'insert-insertannotation-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-page-header',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertPageHeader', 'text'),
								'command': '.uno:InsertPageHeader',
								'accessibility': { focusBack: true,	combination: 'H',	de:	'H' }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-page-footer',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertPageFooter', 'text'),
								'command': '.uno:InsertPageFooter',
								'accessibility': { focusBack: true,	combination: 'O',	de:	null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-page-number-wizard',
								'type': 'toolitem',
								'text': _UNO('.uno:PageNumberWizard', 'text'),
								'command': '.uno:PageNumberWizard',
								'accessibility': { focusBack: false,	combination: 'NU',	de:	null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-field-control',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertFieldCtrl', 'text'),
								'command': '.uno:InsertFieldCtrl',
								'accessibility': { focusBack: false,	combination: 'IE',	de:	null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-title-page-dialog',
								'type': 'toolitem',
								'text': _UNO('.uno:TitlePageDialog', 'text'),
								'command': '.uno:TitlePageDialog',
								'accessibility': { focusBack: false,	combination: 'TI',	de:	null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-section',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertSection', 'text'),
								'command': '.uno:InsertSection',
								'accessibility': { focusBack: false,	combination: 'IS',	de:	null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'insert-insertsection-break', orientation: 'vertical' },
			{
				'id': 'insert-draw-text',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:DrawText'),
				'command': '.uno:DrawText',
				'accessibility': { focusBack: true,	combination: 'X',	de:	null }
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-vertical-text',
								'type': 'toolitem',
								'text': _UNO('.uno:VerticalText', 'text'),
								'command': '.uno:VerticalText',
								'accessibility': { focusBack: false,	combination: 'VT',	de:	null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-line',
								'type': 'toolitem',
								'text': _UNO('.uno:Line', 'text'),
								'command': '.uno:Line',
								'accessibility': { focusBack: true,	combination: 'IL',	de:	null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'insert-insertline-break', orientation: 'vertical' },
			{
				'id': 'insert-font-gallery-floater',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FontworkGalleryFloater'),
				'command': '.uno:FontworkGalleryFloater',
				// Fontwork export/import not supported in other formats.
				'visible': isODF ? 'true' : 'false',
				'accessibility': { focusBack: false,	combination: 'FG',	de:	null }
			},
			{
				'type': 'separator',
				'visible': isODF ? 'true' : 'false',
				'id': 'insert-fontgalleryfloater-break',
				'orienatation': 'vertical'
			},
			{
				'id': 'insert-FormattingMarkMenu:FormattingMarkMenu',
				'type': 'menubutton',
				'text': _UNO('.uno:FormattingMarkMenu', 'text'),
				'command': '.uno:FormattingMarkMenu',
				'accessibility': { focusBack: false,	combination: 'FM',	de: null }
			},
			{ type: 'separator', id: 'insert-formattingmarkmenu-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-QrCode',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertQrCode', 'text'),
								'command': '.uno:InsertQrCode',
								'accessibility': { focusBack: false,	combination: 'IQ',	de: null }
							},
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-frame',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertFrame', 'text'),
								'command': '.uno:InsertFrame',
								'accessibility': { focusBack: false,	combination: 'PT',	de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'insert-insertframe-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-char',
								'class': 'unoCharmapControl',
								'type': 'customtoolitem',
								'text': _UNO('.uno:CharmapControl'),
								'command': 'charmapcontrol',
								'accessibility': { focusBack: false,	combination: 'ZS',	de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-objects-star-math',
								'type': 'toolitem',
								'text': _('Formula'),
								'command': '.uno:InsertObjectStarMath',
								'accessibility': { focusBack: true,	combination: 'ET',	de:	null }
							}
						]
					}
				],
				'vertical': 'true'
			},
		];

		return this.getTabPage(insertTabName, content);
	},

	getFormTab: function() {
		var content = [
			{
				'id': 'form-insert-content-control',
				'type': 'bigtoolitem',
				'text':  _('Rich Text'),
				'command': '.uno:InsertContentControl',
				'accessibility': { focusBack: true, combination: 'A', de: null }
			},
			{ type: 'separator', id: 'form-insertcontentcontrol-break', orientation: 'vertical' },
			{
				'id': 'form-insert-checkbox-control',
				'type': 'bigtoolitem',
				'text': _('Checkbox'),
				'command': '.uno:InsertCheckboxContentControl',
				'accessibility': { focusBack: true, combination: 'B', de: null }
			},
			{ type: 'separator', id: 'form-insertcheckboxcontrol-break', orientation: 'vertical' },
			{
				'id': 'form-insert-dropdown-control',
				'type': 'bigtoolitem',
				'text':  _('Dropdown'),
				'command': '.uno:InsertDropdownContentControl',
				'accessibility': { focusBack: true, combination: 'C', de: null }
			},
			{ type: 'separator', id: 'form-insertdropdowncontrol-break', orientation: 'vertical' },
			{
				'id': 'form-insert-picture-control',
				'type': 'bigtoolitem',
				'text': _('Picture'),
				'command': '.uno:InsertPictureContentControl',
				'accessibility': { focusBack: true, combination: 'D', de: null }
			},
			{ type: 'separator', id: 'form-insertdropdowncontrol-break', orientation: 'vertical' },
			{
				'id': 'form-insert-date-content-control',
				'type': 'bigtoolitem',
				'text': _('Date'),
				'command': '.uno:InsertDateContentControl',
				'accessibility': { focusBack: true, combination: 'E', de: null }
			},
			{ type: 'separator', id: 'form-insertdatecontentcontrol-break', orientation: 'vertical' },
			{
				'id': 'form-content-control-properties',
				'type': 'bigtoolitem',
				'text': _('Properties'),
				'command': '.uno:ContentControlProperties',
				'accessibility': { focusBack: false, combination: 'F', de: null }
			}
		];

		return this.getTabPage(formTabName, content);
	},

	getViewTab: function() {
		var isTablet = window.mode.isTablet();
		var content = [
			isTablet ?
				{
					'id': 'closemobile',
					'type': 'bigcustomtoolitem',
					'text': _('Read mode'),
					'command': 'closetablet',
				} : {},
			{
				'id': 'view-control-codes',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:ControlCodes', 'text'),
				'command': '.uno:ControlCodes',
				'accessibility': { focusBack: true, combination: 'CC', de: null }
			},
			{ type: 'separator', id: 'view-controlcodes-break', orientation: 'vertical' },
			{
				'id': 'fullscreen',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FullScreen'),
				'command': '.uno:FullScreen',
				'accessibility': { focusBack: true, combination: 'F', de: 'E' }
			},
			{
				'id': 'zoomreset',
				'class': 'unozoomreset',
				'type': 'bigcustomtoolitem',
				'text': _('Reset zoom'),
				'accessibility': { focusBack: true, combination: 'J', de: 'O' }
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'zoomout',
								'class': 'unozoomout',
								'type': 'customtoolitem',
								'text': _UNO('.uno:ZoomMinus'),
								'accessibility': { focusBack: true, combination: 'ZO', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'zoomin',
								'class': 'unozoomin',
								'type': 'customtoolitem',
								'text': _UNO('.uno:ZoomPlus'),
								'accessibility': { focusBack: true, combination: 'ZI', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'view-zoomin-break', orientation: 'vertical' },
			{
				'id': 'toggleuimode',
				'class': 'unotoggleuimode',
				'type': 'bigcustomtoolitem',
				'text': _('Compact view'),
				'accessibility': { focusBack: false, combination: 'UI', de: null }
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'showruler',
								'class': 'unoshowruler',
								'type': 'customtoolitem',
								'text': _('Ruler'),
								'accessibility': { focusBack: true, combination: 'R', de: 'L' }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'showstatusbar',
								'class': 'unoshowstatusbar',
								'type': 'customtoolitem',
								'text': _('Status Bar'),
								'accessibility': { focusBack: true, combination: 'AH', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{
				'id': 'collapsenotebookbar',
				'class': 'unocollapsenotebookbar',
				'type': 'bigcustomtoolitem',
				'text': _('Collapse Tabs'),
				'accessibility': { focusBack: true, combination: 'CT', de: null }
			},
			{ type: 'separator', id: 'view-collapsenotebookbar-break', orientation: 'vertical' },
			{
				'id':'toggledarktheme',
				'class': 'unotoggledarktheme',
				'type': 'bigcustomtoolitem',
				'text': _('Dark Mode'),
				'accessibility': { focusBack: true, combination: 'D', de: null }
			},
			{
			    'id':'invertbackground',
			    'class': 'unoinvertbackground',
			    'type': 'bigcustomtoolitem',
			    'text': _('Invert Background'),
			    'accessibility': { focusBack: true, combination: 'BG', de: null }
			},
			{ type: 'separator', id: 'view-invertbackground-break', orientation: 'vertical' },
			{
				'id': 'view-sidebar-property-deck',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:Sidebar'),
				'command': '.uno:SidebarDeck.PropertyDeck',
				'accessibility': { focusBack: true, combination: 'SB', de: null }
			},
			{
				'id': 'view-navigator',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:Navigator'),
				'command': '.uno:Navigator',
				'accessibility': { focusBack: true, combination: 'K', de: 'V' }
			},
		];

		return this.getTabPage(viewTabName, content);
	},

	getLayoutTab: function() {
		var content = [
			{
				'id': 'layout-page-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:PageDialog'),
				'command': '.uno:PageDialog',
				'accessibility': { focusBack: false, combination: 'M', de: '8' }
			},
			{ type: 'separator', id: 'layout-pagedialog-break', orientation: 'vertical' },
			{
				'id': 'layout-format-columns',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FormatColumns', 'text'),
				'command': '.uno:FormatColumns',
				'accessibility': { focusBack: false, combination: 'J', de: 'R' }
			},
			{ type: 'separator', id: 'layout-formatcolumns-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-insert-page-break',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertPagebreak', 'text'),
								'command': '.uno:InsertPagebreak',
								'accessibility': { focusBack: true,	combination: 'IB',	de:	null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-insert-break',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertBreak', 'text'),
								'command': '.uno:InsertBreak',
								'accessibility': { focusBack: false, combination: 'IK', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'layout-insertbreak-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-hyphenate',
								'type': 'toolitem',
								'text':  _UNO('.uno:Hyphenate', 'text'),
								'command': '.uno:Hyphenate',
								'accessibility': { focusBack: true,	combination: 'H', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-line-numbering-dialog',
								'type': 'toolitem',
								'text': _UNO('.uno:LineNumberingDialog', 'text'),
								'command': '.uno:LineNumberingDialog',
								'accessibility': { focusBack: true, combination: 'LN', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'layout-linenumberingdialog-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-title-page-dialog',
								'type': 'toolitem',
								'text': _UNO('.uno:TitlePageDialog', 'text'),
								'command': '.uno:TitlePageDialog',
								'accessibility': { focusBack: true,	combination: 'TP', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-watermark',
								'type': 'toolitem',
								'text': _UNO('.uno:Watermark', 'text'),
								'command': '.uno:Watermark',
								'accessibility': { focusBack: false, combination: 'WM', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'layout-watermark-break', orientation: 'vertical' },
			{
				'id': 'layout-select-all',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:SelectAll'),
				'command': '.uno:SelectAll',
				'accessibility': { focusBack: true,	combination: 'SA', de: null }
			},
			{ type: 'separator', id: 'layout-selectall-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-wrap-off',
								'type': 'toolitem',
								'text': _UNO('.uno:WrapOff', 'text'),
								'command': '.uno:WrapOff',
								'accessibility': { focusBack: true,	combination: 'TW', de: null }
							},
							{
								'id': 'layout-wrap-on',
								'type': 'toolitem',
								'text': _UNO('.uno:WrapOn', 'text'),
								'command': '.uno:WrapOn',
								'accessibility': { focusBack: true,	combination: 'WO', de: null }
							},
							{
								'id': 'layout-wrap-ideal',
								'type': 'toolitem',
								'text': _UNO('.uno:WrapIdeal', 'text'),
								'command': '.uno:WrapIdeal',
								'accessibility': { focusBack: true,	combination: 'WI', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-wrap-left',
								'type': 'toolitem',
								'text': _UNO('.uno:WrapLeft', 'text'),
								'command': '.uno:WrapLeft',
								'accessibility': { focusBack: true,	combination: 'WL', de: null }
							},
							{
								'id': 'layout-wrap-through',
								'type': 'toolitem',
								'text': _UNO('.uno:WrapThrough', 'text'),
								'command': '.uno:WrapThrough',
								'accessibility': { focusBack: true,	combination: 'WT', de: null }
							},
							{
								'id': 'layout-wrap-right',
								'type': 'toolitem',
								'text': _UNO('.uno:WrapRight', 'text'),
								'command': '.uno:WrapRight',
								'accessibility': { focusBack: true,	combination: 'WR', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'layout-wrapright-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-contour-dialog',
								'type': 'toolitem',
								'text': _UNO('.uno:ContourDialog'),
								'command': '.uno:ContourDialog',
								//'accessibility': { focusBack: true,	combination: 'WR', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-text-wrap',
								'type': 'toolitem',
								'text': _UNO('.uno:TextWrap'),
								'command': '.uno:TextWrap',
								//'accessibility': { focusBack: true,	combination: 'WR', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'layout-textwrap-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-object-align-left',
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectAlignLeft', 'text'),
								'command': '.uno:ObjectAlignLeft',
								'accessibility': { focusBack: true,	combination: 'OL', de: null }
							},
							{
								'id': 'layout-align-center',
								'type': 'toolitem',
								'text': _UNO('.uno:AlignCenter', 'text'),
								'command': '.uno:AlignCenter',
								'accessibility': { focusBack: true,	combination: 'AC', de: null }
							},
							{
								'id': 'layout-object-align-right',
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectAlignRight', 'text'),
								'command': '.uno:ObjectAlignRight',
								'accessibility': { focusBack: true,	combination: 'OR', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-align-up',
								'type': 'toolitem',
								'text': _UNO('.uno:AlignUp', 'text'),
								'command': '.uno:AlignUp',
								'accessibility': { focusBack: true,	combination: 'OU', de: null }
							},
							{
								'id': 'layout-align-middle',
								'type': 'toolitem',
								'text': _UNO('.uno:AlignMiddle', 'text'),
								'command': '.uno:AlignMiddle',
								'accessibility': { focusBack: true,	combination: 'AM', de: null }
							},
							{
								'id': 'layout-align-down',
								'type': 'toolitem',
								'text': _UNO('.uno:AlignDown', 'text'),
								'command': '.uno:AlignDown',
								'accessibility': { focusBack: true,	combination: 'AD', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'layout-aligndown-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-object-forward-one',
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectForwardOne', 'text'),
								'command': '.uno:ObjectForwardOne',
								'accessibility': { focusBack: true,	combination: 'OF', de: null }
							},
							{
								'id': 'layout-bring-to-front',
								'type': 'toolitem',
								'text': _UNO('.uno:BringToFront', 'text'),
								'command': '.uno:BringToFront',
								'accessibility': { focusBack: true, combination: 'BF', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'layout-object-back-one',
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectBackOne', 'text'),
								'command': '.uno:ObjectBackOne',
								'accessibility': { focusBack: true,	combination: 'OB', de: null }
							},
							{
								'id': 'layout-send-to-back',
								'type': 'toolitem',
								'text': _UNO('.uno:SendToBack', 'text'),
								'command': '.uno:SendToBack',
								'accessibility': { focusBack: true, combination: 'SB', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			}
		];

		return this.getTabPage(layoutTabName, content);
	},

	getReferencesTab: function() {
		var content = [
			{
				'id': 'references-insert-multi-index',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:IndexesMenu', 'text'),
				'command': '.uno:InsertMultiIndex',
				'accessibility': { focusBack: false, combination: 'T', de: 'LA' }
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'references-insert-indexes-entry',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertIndexesEntry', 'text'),
								'command': '.uno:InsertIndexesEntry',
								'accessibility': { focusBack: false, combination: 'IE', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'references-update-current-index',
								'type': 'toolitem',
								'text': _('Update Index'),
								'command': '.uno:UpdateCurIndex',
								'accessibility': { focusBack: false, combination: 'UI', de: 'T' }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'references-updatecurindex-break', orientation: 'vertical' },
			{
				'id': 'references-insert-foot-note',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:InsertFootnote', 'text'),
				'command': '.uno:InsertFootnote',
				'accessibility': { focusBack: true, combination: 'F', de: 'U' }
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'references-insert-end-note',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertEndnote', 'text'),
								'command': '.uno:InsertEndnote',
								'accessibility': { focusBack: true, combination: 'E', de: 'E' }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'references-foot-note-dialog',
								'type': 'toolitem',
								'text': _UNO('.uno:FootnoteDialog', 'text'),
								'command': '.uno:FootnoteDialog',
								'accessibility': { focusBack: false, combination: 'H', de: 'I' }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'references-footnotedialog-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'references-insert-bookmark',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertBookmark', 'text'),
								'command': '.uno:InsertBookmark',
								'accessibility': { focusBack: false, combination: 'IB', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'references-insert-reference-field',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertReferenceField', 'text'),
								'command': '.uno:InsertReferenceField',
								'accessibility': { focusBack: false, combination: 'IR', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'references-insertreferencefield-break', orientation: 'vertical' }
		];
		if (this.map.zotero) {
			content.push(
				{
					'id': 'zoteroaddeditbibliography',
					'class': 'unozoteroaddeditbibliography',
					'type': 'bigcustomtoolitem',
					'text': _('Add Bibliography'),
					'command': 'zoteroaddeditbibliography',
					'accessibility': { focusBack: true, combination: 'AB', de: null }
				},
				{
					'type': 'container',
					'children': [
						{
							'type': 'toolbox',
							'children': [
								{
									'id': 'zoteroAddEditCitation',
									'class': 'unozoteroAddEditCitation',
									'type': 'customtoolitem',
									'text': _('Add Citation'),
									'command': 'zoteroaddeditcitation',
									'accessibility': { focusBack: true, combination: 'AC', de: null }
								}
							]
						},
						{
							'type': 'toolbox',
							'children': [
								{
									'id': 'zoteroaddnote',
									'class': 'unozoteroaddnote',
									'type': 'customtoolitem',
									'text': _('Add Citation Note'),
									'command': 'zoteroaddnote',
									'accessibility': { focusBack: true, combination: 'CN', de: null }
								}
							]
						}
					],
					'vertical': 'true'
				},
				{
					'type': 'container',
					'children': [
						{
							'type': 'toolbox',
							'children': [
								{
									'id': 'zoterorefresh',
									'class': 'unozoterorefresh',
									'type': 'customtoolitem',
									'text': _('Refresh Citations'),
									'command': 'zoterorefresh',
									'accessibility': { focusBack: true, combination: 'RC', de: null }
								}
							]
						},
						{
							'type': 'toolbox',
							'children': [
								{
									'id': 'zoterounlink',
									'class': 'unozoterounlink',
									'type': 'customtoolitem',
									'text': _('Unlink Citations'),
									'command': 'zoterounlink',
									'accessibility': { focusBack: true, combination: 'UC', de: null }
								}
							]
						}
					],
					'vertical': 'true'
				},
				{
					'id': 'zoteroSetDocPrefs',
					'class': 'unozoteroSetDocPrefs',
					'type': 'bigcustomtoolitem',
					'text': _('Citation Preferences'),
					'command': 'zoterosetdocprefs',
					'accessibility': { focusBack: true, combination: 'CP', de: null }
				},
				{ type: 'separator', id: 'references-zoterosetdocprefs-break'}
			);
		}

		content.push(
			{
				'id': 'references-insert-field-control',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:InsertFieldCtrl', 'text'),
				'command': '.uno:InsertFieldCtrl',
				'accessibility': { focusBack: false, combination: 'IF', de: null }
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'references-inset-page-number-field',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertPageNumberField'),
								'command': '.uno:InsertPageNumberField',
								'accessibility': { focusBack: true, combination: 'PN', de: null }
							},
							{
								'id': 'references-insert-page-count-field',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertPageCountField', 'text'),
								'command': '.uno:InsertPageCountField',
								'accessibility': { focusBack: true, combination: 'PC', de: null }
							},
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'references-insert-date-field',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertDateField', 'text'),
								'command': '.uno:InsertDateField',
								'accessibility': { focusBack: true, combination: 'ID', de: null }
							},
							{
								'id': 'references-insert-title-field',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertTitleField', 'text'),
								'command': '.uno:InsertTitleField',
								'accessibility': { focusBack: true, combination: 'IT', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'references-inserttitlefield-break', orientation: 'vertical' },
			{
				'id': 'references-update-all',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:UpdateAll', 'text'),
				'command': '.uno:UpdateAll',
				'accessibility': { focusBack: true, combination: 'UA', de: null }
			}
		);

		return this.getTabPage(referencesTabName, content);
	},

	getReviewTab: function() {
		var content = [
			{
				'id': 'review-spelling-and-grammar-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:SpellingAndGrammarDialog'),
				'command': '.uno:SpellingAndGrammarDialog',
				'accessibility': { focusBack: false, combination: 'SP', de: 'C' }
			},
			{
				'id': 'review-thesaurus-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:ThesaurusDialog'),
				'command': '.uno:ThesaurusDialog',
				'accessibility': { focusBack: false, combination: 'E', de: null }
			},
			{
				'id': 'LanguageMenu:LanguageMenu',
				'type': 'menubutton',
				'text': _UNO('.uno:LanguageMenu'),
				'command': '.uno:LanguageMenu',
				'accessibility': { focusBack: false, combination: 'ZL', de: null }
			},
			window.deeplEnabled ?
				{
					'id': 'review-translate',
					'type': 'bigtoolitem',
					'text': _UNO('.uno:Translate', 'text'),
					'command': '.uno:Translate'
				}: {},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-spell-online',
								'type': 'toolitem',
								'text': _UNO('.uno:SpellOnline'),
								'command': '.uno:SpellOnline',
								'accessibility': { focusBack: true, combination: 'SO', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-word-count-dialog',
								'type': 'toolitem',
								'text': _UNO('.uno:WordCountDialog', 'text'),
								'command': '.uno:WordCountDialog',
								'accessibility': { focusBack: false, combination: 'W', de: 'W' }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'review-wordcountdialog-break', orientation: 'vertical' },
			{
				'id': 'review-insert-annotation',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:InsertAnnotation'),
				'command': '.uno:InsertAnnotation',
				'accessibility': { focusBack: false, combination: 'C', de: 'N' }
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'showannotations',
								'type': 'customtoolitem',
								'text': _UNO('.uno:ShowAnnotations', 'text'),
								'command': 'showannotations',
								'accessibility': { focusBack: true, combination: 'SA', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-show-resolved-annotations',
								'type': 'toolitem',
								'text': _UNO('.uno:ShowResolvedAnnotations', 'text'),
								'command': '.uno:ShowResolvedAnnotations',
								'accessibility': { focusBack: true, combination: 'RA', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-reply-comment',
								'type': 'toolitem',
								'text': _UNO('.uno:ReplyComment'),
								'command': '.uno:ReplyComment'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-delete-comment',
								'type': 'toolitem',
								'text': _UNO('.uno:DeleteComment'),
								'command': '.uno:DeleteComment'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'review-deletecomment-break', orientation: 'vertical' },
			{
				'id': 'review-track-changes:RecordTrackedChangesMenu',
				'type': 'menubutton',
				'text': _UNO('.uno:TrackChanges', 'text'),
				'command': '.uno:TrackChanges',
				'accessibility': { focusBack: true, combination: 'TC', de: null }
			},
			{
				'id': 'review-show-tracked-changes',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:ShowTrackedChanges', 'text'),
				'command': '.uno:ShowTrackedChanges',
				'accessibility': { focusBack: true, combination: 'SC', de: null }
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-next-tracked-change',
								'type': 'toolitem',
								'text': _UNO('.uno:NextTrackedChange', 'text'),
								'command': '.uno:NextTrackedChange',
								'accessibility': { focusBack: true, combination: 'H', de: 'H' }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-previous-tracked-change',
								'type': 'toolitem',
								'text': _UNO('.uno:PreviousTrackedChange', 'text'),
								'command': '.uno:PreviousTrackedChange',
								'accessibility': { focusBack: true, combination: 'F', de: 'F' }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-accept-tracked-change',
								'type': 'toolitem',
								'text': _UNO('.uno:AcceptTrackedChange', 'text'),
								'command': '.uno:AcceptTrackedChange',
								'accessibility': { focusBack: true, combination: 'AC', de: 'AC' }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-reject-tracked-change',
								'type': 'toolitem',
								'text': _UNO('.uno:RejectTrackedChange', 'text'),
								'command': '.uno:RejectTrackedChange',
								'accessibility': { focusBack: true, combination: 'JR', de: 'JR' }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-accept-tracked-change-to-next',
								'type': 'toolitem',
								'text': _UNO('.uno:AcceptTrackedChangeToNext', 'text'),
								'command': '.uno:AcceptTrackedChangeToNext',
								'accessibility': { focusBack: true, combination: 'AM', de: 'AM' }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-reject-tracked-change-to-next',
								'type': 'toolitem',
								'text': _UNO('.uno:RejectTrackedChangeToNext', 'text'),
								'command': '.uno:RejectTrackedChangeToNext',
								'accessibility': { focusBack: true, combination: 'JM', de: 'JM' }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'acceptalltrackedchanges',
								'type': 'customtoolitem',
								'text': _UNO('.uno:AcceptAllTrackedChanges', 'text'),
								'command': '.uno:AcceptAllTrackedChanges',
								'accessibility': { focusBack: true, combination: 'AL', de: 'AL' }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'rejectalltrackedchanges',
								'type': 'customtoolitem',
								'text': _UNO('.uno:RejectAllTrackedChanges', 'text'),
								'command': '.uno:RejectAllTrackedChanges',
								'accessibility': { focusBack: true, combination: 'JL', de: 'JL' }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-reinstate-tracked-change',
								'type': 'toolitem',
								'text': _UNO('.uno:ReinstateTrackedChange', 'text'),
								'command': '.uno:ReinstateTrackedChange',
								'accessibility': { focusBack: true, combination: 'RR' }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-accept-tracked-changes',
								'type': 'toolitem',
								'text': _UNO('.uno:AcceptTrackedChanges', 'text'),
								'command': '.uno:AcceptTrackedChanges',
								'accessibility': { focusBack: false, combination: 'AA', de: null }
							}
						]
					},
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'review-accepttrackedchanges-break', orientation: 'vertical' },
			{
				'id': 'review-accessibility-check',
				'class': 'unoAccessibilityCheck',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:AccessibilityCheck', 'text'),
				'command': '.uno:SidebarDeck.A11yCheckDeck',
				'accessibility': { focusBack: false, combination: 'A1', de: 'B' }
			}
		];

		return this.getTabPage(reviewTabName, content);
	},

	getTableTab: function() {
		var content = [
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TableDialog', 'text', true),
				'command': '.uno:TableDialog'
			},
			{ type: 'separator', id: 'table-bigtoolitem-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:InsertColumnsBefore', 'text', true),
								'command': '.uno:InsertColumnsBefore'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:InsertColumnsAfter', 'text', true),
								'command': '.uno:InsertColumnsAfter'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:DeleteColumns', 'text', true),
								'command': '.uno:DeleteColumns'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:InsertRowsBefore', 'text', true),
								'command': '.uno:InsertRowsBefore'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:InsertRowsAfter', 'text', true),
								'command': '.uno:InsertRowsAfter'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:DeleteRows', 'text', true),
								'command': '.uno:DeleteRows'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'table-deleterows-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:MergeCells', 'text'),
				'command': '.uno:MergeCells'
			},
			{ type: 'separator', id: 'table-mergecells-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:SplitCell', 'text'),
								'command': '.uno:SplitCell'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:SplitTable', 'text'),
								'command': '.uno:SplitTable'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'table-splittable-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:Protect', 'text'),
								'command': '.uno:Protect'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:UnsetCellsReadOnly', 'text'),
								'command': '.uno:UnsetCellsReadOnly'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'table-unsetcellsreadonly-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:EntireCell', 'text', true),
				'command': '.uno:EntireCell'
			},
			{ type: 'separator', id: 'table-entirecell-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:EntireColumn', 'presentation'),
								'command': '.uno:EntireColumn'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:SelectTable', 'text', true),
								'command': '.uno:SelectTable'
							},
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:EntireRow', 'presentation'),
								'command': '.uno:EntireRow'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:DeleteTable', 'text', true),
								'command': '.uno:DeleteTable'
							},
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'table-deletetable-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:CellVertTop'),
								'command': '.uno:CellVertTop'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:CellVertCenter'),
								'command': '.uno:CellVertCenter'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:CellVertBottom'),
								'command': '.uno:CellVertBottom'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:LeftPara'),
								'command': '.uno:LeftPara'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:CenterPara'),
								'command': '.uno:CenterPara'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:RightPara'),
								'command': '.uno:RightPara'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:JustifyPara'),
								'command': '.uno:JustifyPara'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'table-justifypara-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TableSort', 'text'),
				'command': '.uno:TableSort'
			},
			{ type: 'separator', id: 'table-tablesort-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TableNumberFormatDialog', 'text'),
				'command': '.uno:TableNumberFormatDialog'
			},
			{ type: 'separator', id: 'table-tablenumberformatdialog-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:NumberFormatCurrency', 'text'),
								'command': '.uno:NumberFormatCurrency'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:NumberFormatDate', 'text'),
								'command': '.uno:NumberFormatDate'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:TableCellBackgroundColor', 'text'),
								'command': '.uno:TableCellBackgroundColor',
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:NumberFormatPercent', 'text'),
								'command': '.uno:NumberFormatPercent'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'table-numberformatpercent-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:InsertCaptionDialog', 'text'),
				'command': '.uno:InsertCaptionDialog'
			},
		];

		return this.getTabPage(tableTabName, content);
	},

	getFormulaTab: function() {
		var content = [
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:ChangeFont', 'text'),
				'command': '.uno:ChangeFont',
				'icon': 'lc_fontdialog.svg'
			},
			{ type: 'separator', id: 'formula-changefont-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:ChangeFontSize', 'text'),
				'command': '.uno:ChangeFontSize',
				'icon': 'lc_fontheight.svg'
			},
			{ type: 'separator', id: 'formula-changefontsize-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:ChangeDistance', 'text'),
				'command': '.uno:ChangeDistance',
				'icon': 'lc_spacing.svg',
			},
			{ type: 'separator', id: 'formula-changedistance-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:ChangeAlignment', 'text'),
				'command': '.uno:ChangeAlignment',
				'icon': 'lc_fontworkalignmentfloater.svg'
			}
        ];
		return this.getTabPage(formulaTabName, content);
	},

	getShapeTab: function() {
		var isODF = app.LOUtil.isFileODF(this.map);
		var content = [
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TransformDialog', 'text'),
				'command': '.uno:TransformDialog'
			},
			{ type: 'separator', id: 'shape-transformdialog-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:FlipVertical'),
								'command': '.uno:FlipVertical'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:FlipHorizontal'),
								'command': '.uno:FlipHorizontal'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'shape-fliphorizontal-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'shape-tab-xlinecolor:ColorPickerMenu',
								'noLabel': true,
								'type': 'toolitem',
								'text': _UNO('.uno:XLineColor'),
								'command': '.uno:XLineColor'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'shape-tab-fillcolor:ColorPickerMenu',
								'noLabel': true,
								'type': 'toolitem',
								'text': _UNO('.uno:FillColor'),
								'command': '.uno:FillColor'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'shape-fillcolor-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:WrapOff', 'text'),
								'command': '.uno:WrapOff'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:WrapOn', 'text'),
								'command': '.uno:WrapOn'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:WrapIdeal', 'text'),
								'command': '.uno:WrapIdeal'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:WrapLeft', 'text'),
								'command': '.uno:WrapLeft'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:WrapThrough', 'text'),
								'command': '.uno:WrapThrough'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:WrapRight', 'text'),
								'command': '.uno:WrapRight'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'shape-wrapright-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectAlignLeft'),
								'command': '.uno:ObjectAlignLeft'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:AlignCenter'),
								'command': '.uno:AlignCenter'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectAlignRight'),
								'command': '.uno:ObjectAlignRight'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:AlignUp'),
								'command': '.uno:AlignUp'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:AlignMiddle'),
								'command': '.uno:AlignMiddle'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:AlignDown'),
								'command': '.uno:AlignDown'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'shape-aligndown-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:BringToFront'),
								'command': '.uno:BringToFront'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:SendToBack'),
								'command': '.uno:SendToBack'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectForwardOne'),
								'command': '.uno:ObjectForwardOne'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectBackOne'),
								'command': '.uno:ObjectBackOne'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'shape-objectbackone-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FormatGroup'),
				'command': '.uno:FormatGroup'
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:EnterGroup'),
								'command': '.uno:EnterGroup'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:LeaveGroup'),
								'command': '.uno:LeaveGroup'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'shape-leavegroup-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:Text'),
				'command': '.uno:Text'
			},
			{ type: 'separator', id: 'shape-text-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'shape-tab-shapes:InsertShapesMenu',
								'type': 'menubutton',
								'noLabel': true,
								'text': _('Shapes'),
								'command': '.uno:BasicShapes'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:Line', 'text'),
								'command': '.uno:Line'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'shape-line-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:FontworkGalleryFloater'),
								'command': '.uno:FontworkGalleryFloater',
								// Fontwork export/import not supported in other formats.
								'visible': isODF ? 'true' : 'false',
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:VerticalText', 'text'),
								'command': '.uno:VerticalText'
							}
						]
					}
				],
				'vertical': 'true'
			},
		];

		return this.getTabPage(shapeTabName, content);
	},

	getPictureTab: function() {
		var content = [
			{
				'type': 'container',
				'children': [
					{
						'id': 'picture-brightness:PictureBrightness',
						'type': 'menubutton',
						'command': '.uno:GrafLuminance',
						'text': _UNO('.uno:GrafLuminance'),
						'icon': 'lc_setbrightness.svg',
						'accessibility': { focusBack: true, combination: 'BN', de: null }
					},
					{
						'id': 'picture-contrast:PictureContrast',
						'type': 'menubutton',
						'command': '.uno:GrafContrast',
						'text': _UNO('.uno:GrafContrast'),
						'icon': 'lc_setcontrast.svg',
						'accessibility': { focusBack: true, combination: 'CN', de: null }
					},
					{
						'id': 'picture-colormode:PictureColorMode',
						'type': 'menubutton',
						'command': '.uno:GrafMode',
						'text': _UNO('.uno:GrafMode'),
						'icon': 'lc_setgraphtransparency.svg',
						'accessibility': { focusBack: true, combination: 'CO', de: null }
					},
					{
						'id': 'picture-transparency:PictureTransparency',
						'type': 'menubutton',
						'command': '.uno:GrafTransparence',
						'text': _UNO('.uno:GrafTransparence'),
						'icon': 'lc_setgraphtransparency.svg',
						'accessibility': { focusBack: true, combination: 'TP', de: null }
					},
				]
			},
			{ type: 'separator', id: 'picture-transparency-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'picture-tab-xlinecolor:ColorPickerMenu',
								'type': 'menubutton',
								'noLabel': true,
								'text': _UNO('.uno:XLineColor'),
								'command': '.uno:XLineColor'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'picture-tab-fillcolor:ColorPickerMenu',
								'type': 'menubutton',
								'noLabel': true,
								'text': _UNO('.uno:FillColor'),
								'command': '.uno:FillColor'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'picture-fillcolor-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TransformDialog', 'text'),
				'command': '.uno:TransformDialog'
			},
			{ type: 'separator', id: 'picture-transformdialog-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:FlipVertical'),
								'command': '.uno:FlipVertical'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:FlipHorizontal'),
								'command': '.uno:FlipHorizontal'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'picture-fliphorizontal-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:WrapOff', 'text'),
								'command': '.uno:WrapOff'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:WrapOn', 'text'),
								'command': '.uno:WrapOn'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:WrapIdeal', 'text'),
								'command': '.uno:WrapIdeal'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:WrapLeft', 'text'),
								'command': '.uno:WrapLeft'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:WrapThrough', 'text'),
								'command': '.uno:WrapThrough'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:WrapRight', 'text'),
								'command': '.uno:WrapRight'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'picture-wrapright-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectAlignLeft'),
								'command': '.uno:ObjectAlignLeft'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:AlignCenter'),
								'command': '.uno:AlignCenter'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectAlignRight'),
								'command': '.uno:ObjectAlignRight'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:AlignUp'),
								'command': '.uno:AlignUp'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:AlignMiddle'),
								'command': '.uno:AlignMiddle'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:AlignDown'),
								'command': '.uno:AlignDown'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'picture-aligndown-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:BringToFront'),
								'command': '.uno:BringToFront'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:SendToBack'),
								'command': '.uno:SendToBack'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectForwardOne'),
								'command': '.uno:ObjectForwardOne'
							},
							{
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectBackOne'),
								'command': '.uno:ObjectBackOne'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'picture-objectbackone-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FormatGroup'),
				'command': '.uno:FormatGroup'
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:EnterGroup'),
								'command': '.uno:EnterGroup'
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'type': 'toolitem',
								'text': _UNO('.uno:LeaveGroup'),
								'command': '.uno:LeaveGroup'
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'picture-leavegroup-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:Crop'),
				'command': '.uno:Crop',
				'context': 'Graphic'
			},
		];

		return this.getTabPage(pictureTabName, content);
	},

	getNotebookbar: function(tabPages, selectedPage) {
		return {
			'id': '',
			'type': 'control',
			'text': '',
			'enabled': 'true',
			'children': [
				{
					'id': 'NotebookBar',
					'type': 'container',
					'text': '',
					'enabled': 'true',
					'children': [
						{
							'id': 'ContextContainer',
							'type': 'tabcontrol',
							'noCoreEvents': true,
							'text': '',
							'enabled': 'true',
							'selected': selectedPage,
							'children': tabPages
						}
					]
				}
			]
		};
	},

	// filter out empty children options so that the HTML isn't cluttered
	// and individual items missaligned
	// Also remove the hidden items / commands.
	cleanOpts: function(children) {
		var that = this;

		return children.map(function(c) {
			if (!c.type) {
				return null;
			}

			var uiManager = that.map.uiManager;
			if (!uiManager.isButtonVisible(c.id)) {
				return null;
			}
			if (!uiManager.isCommandVisible(c.command)) {
				return null;
			}

			var opts = Object.assign(c, {});

			if (c.children && c.children.length) {
				opts.children = that.cleanOpts(c.children);
			}

			return opts;
		}).filter(function(c) {
			return c !== null;
		});
	},

	getTabPage: function(tabName, content) {
		return {
			'id': '',
			'type': 'tabpage',
			'text': '',
			'enabled': 'true',
			'children': [
				{
					'id': tabName + '-container',
					'type': 'container',
					'text': '',
					'enabled': 'true',
					'children': this.cleanOpts(content)
				}
			]
		};
	}
});

L.control.notebookbarWriter = function (options) {
	return new L.Control.NotebookbarWriter(options);
};
