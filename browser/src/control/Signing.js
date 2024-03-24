/* -*- js-indent-level: 8 -*- */
/*
 * Document Signing
 */

/* global _ app w2ui*/

L.Map.include({
	showSignDocument: function() {
		$('#document-signing-bar').show();
		this.initializeLibrary();
		oldtoolbarSize = $(this.options.documentContainer).css('top');

		$(this.options.documentContainer).css('top', '116px');

		// Avoid scroll button ">>"
		var el = w2ui['document-signing-bar'];
		if (el)
			el.resize();
	},
	hideSignToolbar: function() {
		$('#document-signing-bar').hide();
		library = null;
		identity = null;
		currentPassport = null;
		$(this.options.documentContainer).css('top', oldtoolbarSize);
	},
	signingInitializeBar: function() {
		$('#document-signing-bar').hide();
		adjustUIState();
	},
	signingLogout: function() {
		if (library) {
			library.logout().then(function() {
				identity = null;
				currentPassport = null;
				updateIndentity();
				adjustUIState();
			});
		}
	},
	signingLogin: function() {
		var w = window.innerWidth / 2;

		var loginWithQR = false;
		var recoverFromEmail = false;
		var selectedIdentityKey = null;

		$.get('signing-identities.html', function(data) {
			vex.open({
				content: data,
				showCloseButton: true,
				escapeButtonCloses: true,
				overlayClosesOnClick: true,
				buttons: {},
				afterOpen: function() {
					var that = this;
					this.contentEl.style.width = w + 'px';
					var $vexContent = $(this.contentEl);
					$('#select-identity').text(_('Select identity:'));
					$('#login-qr').text(_('Login from mobile'));
					$('#recover-from-email').text(_('Recover from email'));
					library.listIdentities().then(function(response) {
						var identities = response.data;
						var identitiesDiv = $vexContent.find('#identities');
						for (var key in identities) {
							var button = $('<input class="identity-button" type="button"/>');
							button.attr('value', identities[key].initials);
							button.css('background-color', identities[key].identityColor);
							button.click({ key: key }, function(current) {
								selectedIdentityKey = current.data.key;
								that.close();
							});
							identitiesDiv.append(button);
						}
					});
					$('#login-qr').click(function() {
						loginWithQR = true;
						that.close();
					});
					$('#recover-from-email').click(function() {
						recoverFromEmail = true;
						that.close();
					});
				},
				afterClose: function () {
					if (loginWithQR) {
						verignQrDialog();
					}
					else if (recoverFromEmail) {
						vereignRecoverFromEmailDialog();
					}
					else if (selectedIdentityKey) {
						vereignLoadIdentity(selectedIdentityKey, '00000000');
					}
				}
			});
		});
	},
	initializeLibrary: function() {
		var vereignURL = window.documentSigningURL == null ? '' : window.documentSigningURL;
		if (vereignURL.length == 0)
			return;
		_map = this;
		setupViamAPI(
			'signdocument-iframe-content',
			{
				onEvent: function(event) {
					switch (event.type) {
					case 'ActionConfirmedAndExecuted':
						window.app.console.log('event ActionConfirmedAndExecuted');
						break;
					case 'IdentityNotLoaded':
						vereignLoadIdentity(event.payloads[0], '00000000');
						break;
					case 'Authenticated':
						window.app.console.log('event Authenticated');
						vereignRestoreIdentity();
						break;
					case 'Logout':
						window.app.console.log('event Logout');
						_map.signingLogout();
						break;
					case 'QRCodeUpdated':
						window.app.console.log('event QRCodeUpdated');
						break;
					default:
						window.app.console.log('UNKNOWN EVENT: ' + event.type);
						break;
					}
				}
			},
			getVereignIFrameURL(),
			getVereignApiURL(),
			getVereignWopiURL()
		).then(function(lib) {
			library = lib;
			adjustUIState();
		});
	},
	setCurrentPassport: function(uuid, text) {
		if (library && identity && uuid) {
			currentPassport = { uuid: uuid, text: text };
			updateCurrentPassport();
			library.passportGetAvatarByPassport(uuid).then(function(result) {
				window.app.console.log(result); // TODO
			});
			adjustUIState();
		}
	},
	handleSigningClickEvent: function(id, item) {
		if (id === 'close-document-signing-bar') {
			this.signingLogout();
			this.hideSignToolbar();
		}
		else if (id === 'login' || id === 'identity') {
			this.signingLogin();
		}
		else if (id === 'sign-upload') {
			vereignSignAndUploadDocument();
		}
		else if (id.startsWith('passport:')) {
			this.setCurrentPassport(item.value, item.text);
		}
		return false;
	},
	setupSigningToolbarItems: function() {
		return [
			{type: 'html',  id: 'logo', html: '<a href="http://www.vereign.com" target="_blank"><img src="' + L.LOUtil.getImageURL('vereign.png') + '" style="padding-right: 16px; padding-left: 6px; height: 32px"/></a>' },
			{type: 'menu', id: 'passport', caption: _('Select passport'), items: []},
			{type: 'html', id: 'current-passport', html: _('Passport: N/A')},
			{type: 'break', id: 'passport-break' },
			{type: 'button',  id: 'sign-upload',  caption: _('Sign'), img: '', hint: _('Sign document')},
			{type: 'break', id: 'sign-upload-break' },
			{type: 'html', id: 'current-document-status-label', html: '<p><b>' + _('Status:') + '&nbsp;</b></p>'},
			{type: 'html', id: 'current-document-status', html: _('N/A')},
			{type: 'spacer'},
			{type: 'html', id: 'identity', html: ''},
			{type: 'button',  id: 'login',  caption: _('Login'), img: '', hint: _('Login')},
			{type: 'button',  id: 'close-document-signing-bar', img: 'closetoolbar', hint: _('Close')}];
	},
	onChangeSignStatus: function(signstatus) {
		var statusText = '';
		var statusIcon = '';
		// This is meant to be in sync with core.git
		// include/sfx2/signaturestate.hxx, SignatureState.
		switch (signstatus) {
		case '0':
			currentDocumentSigningStatus = _('Not Signed');
			break;
		case '1':
			statusText = _('This document is digitally signed and the signature is valid.');
			statusIcon = 'sign_ok';
			currentDocumentSigningStatus = _('Signed and validated');
			break;
		case '2':
			statusText = _('This document has an invalid signature.');
			statusIcon = 'sign_not_ok';
			currentDocumentSigningStatus = _('Signature broken');
			break;
		case '3':
			statusText = _('The signature was valid, but the document has been modified.');
			statusIcon = 'sign_not_ok';
			currentDocumentSigningStatus = _('Signed but document modified');
			break;
		case '4':
			statusText = _('The signature is OK, but the certificate could not be validated.');
			statusIcon = 'sign_not_ok';
			currentDocumentSigningStatus = _('Signed but not validated');
			break;
		case '5':
			statusText = _('The signature is OK, but the document is only partially signed.');
			statusIcon = 'sign_not_ok';
			currentDocumentSigningStatus = _('Signed but not all files are signed');
			break;
		case '6':
			statusText = _('The signature is OK, but the certificate could not be validated and the document is only partially signed.');
			statusIcon = 'sign_not_ok';
			currentDocumentSigningStatus = _('Signed but not validated and not all files are signed');
			break;
		}

		if (statusText) {
			if (!window.mode.isMobile())
				app.map.statusBar.showSigningItem(statusIcon, statusText);
			else
				w2ui['actionbar'].insert('undo', {type: 'button',  id: 'signstatus', img: statusIcon, hint: statusText});
		}

		adjustUIState();

		if (awaitForDocumentStatusToUpload) {
			awaitForDocumentStatusToUpload = false;
			vereignUpload(currentDocumentType);
		}
		awaitForDocumentStatusToUpload = false;
		currentDocumentType = null;
	},
	onVereignUploadStatus: function(uploadStatus) {
		if (uploadStatus == 'OK') {
			_map.fire('infobar', {
				msg: _('Document uploaded.'),
				action: null,
				actionLabel: null
			});
		}
	}
});
