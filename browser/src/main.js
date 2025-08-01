/* -*- js-indent-level: 8 -*- */

/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* global errorMessages accessToken accessTokenTTL noAuthHeader accessHeader createOnlineModule */
/* global app $ L host idleTimeoutSecs outOfFocusTimeoutSecs _ LocaleService LayoutingService */
/*eslint indent: [error, "tab", { "outerIIFEBody": 0 }]*/

(function (global) {


var wopiParams = {};
var wopiSrc = global.loolParams.get('WOPISrc');

if (wopiSrc !== '' && accessToken !== '') {
	wopiParams = { 'access_token': accessToken, 'access_token_ttl': accessTokenTTL, 'no_auth_header': noAuthHeader };
}
else if (wopiSrc !== '' && accessHeader !== '') {
	wopiParams = { 'access_header': accessHeader };
}

if (window.ThisIsTheEmscriptenApp)
	// Temporary hack
	var filePath = 'file:///sample.docx';
else
	var filePath = global.loolParams.get('file_path');

app.localeService = new LocaleService();
app.setPermission(global.loolParams.get('permission') || 'edit');
app.layoutingService = new LayoutingService();

app.setPermission(global.loolParams.get('permission') || 'edit');

var timestamp = global.loolParams.get('timestamp');
var target = global.loolParams.get('target') || '';
// Should the document go inactive or not
var alwaysActive = global.loolParams.get('alwaysactive');
// Lool Debug mode
var debugMode = global.loolParams.get('debug');

var docURL, docParams;
var isWopi = false;
if (wopiSrc != '') {
	docURL = decodeURIComponent(wopiSrc);
	docParams = wopiParams;
	isWopi = true;
} else {
	docURL = filePath;
	docParams = {};
}

var notWopiButIframe = global.loolParams.get('NotWOPIButIframe') != '';
var map = L.map('map', {
	server: host,
	doc: docURL,
	docParams: docParams,
	timestamp: timestamp,
	docTarget: target,
	documentContainer: 'document-container',
	debug: debugMode,
	// the wopi and wopiSrc properties are in sync: false/true : empty/non-empty
	wopi: isWopi,
	wopiSrc: wopiSrc,
	notWopiButIframe: notWopiButIframe,
	alwaysActive: alwaysActive,
	idleTimeoutSecs: idleTimeoutSecs,  // Dim when user is idle.
	outOfFocusTimeoutSecs: outOfFocusTimeoutSecs, // Dim after switching tabs.
});

////// Controls /////

map.uiManager = L.control.uiManager();
map.addControl(map.uiManager);
if (!L.Browser.cypressTest)
	map.tooltip = L.control.tooltip();

map.uiManager.initializeBasicUI();

if (wopiSrc === '' && filePath === '' && !window.ThisIsAMobileApp) {
	map.uiManager.showInfoModal('wrong-wopi-src-modal', '', errorMessages.wrongwopisrc, '', _('OK'), null, false);
}
if (host === '' && !window.ThisIsAMobileApp) {
	map.uiManager.showInfoModal('empty-host-url-modal', '', errorMessages.emptyhosturl, '', _('OK'), null, false);
}

L.Map.THIS = map;
app.map = map;
app.idleHandler.map = map;

if (window.ThisIsTheEmscriptenApp) {
	var docParamsString = $.param(docParams);
	// The URL may already contain a query (e.g., 'http://server.tld/foo/wopi/files/bar?desktop=baz') - then just append more params
	var docParamsPart = docParamsString ? (docURL.includes('?') ? '&' : '?') + docParamsString : '';
	var encodedWOPI = encodeURIComponent(docURL + docParamsPart);

	window.Module = {
		onRuntimeInitialized: function() {
			map.loadDocument(global.socket);
		},
		print: function (text) {
			if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
			console.warn(text);
		},
		printErr: function (text) {
			if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
			console.error(text);
		},
		arguments_: [docURL, encodedWOPI, isWopi ? 'true' : 'false'],
		arguments: [docURL, encodedWOPI, isWopi ? 'true' : 'false'],
	};
	createOnlineModule(window.Module).then(() => {
		app.HandleLOOLMessage = window.Module['_handle_lool_message'];
		app.AllocateUTF8 = window.Module['stringToNewUTF8']; });
} else {
	map.loadDocument(global.socket);
}

window.addEventListener('beforeunload', function () {
	if (map && app.socket) {
		if (app.socket.setUnloading)
			app.socket.setUnloading();
		app.socket.close();
	}
});

window.bundlejsLoaded = true;


////// Unsupported Browser Warning /////

var uaLowerCase = navigator.userAgent.toLowerCase();
if (uaLowerCase.indexOf('msie') != -1 || uaLowerCase.indexOf('trident') != -1) {
	map.uiManager.showInfoModal(
		'browser-not-supported-modal', '',
		_('Warning! The browser you are using is not supported.'),
		'', _('OK'), null, false);
}

}(window));
