m4_dnl -*- Mode: HTML -*-x
m4_changequote([,])m4_dnl
m4_dnl# m4_foreachq(x, `item_1, item_2, ..., item_n', stmt)
m4_dnl# quoted list, alternate improved version
m4_define([m4_foreachq],[m4_ifelse([$2],[],[],[m4_pushdef([$1])_$0([$1],[$3],[],$2)m4_popdef([$1])])])m4_dnl
m4_define([_m4_foreachq],[m4_ifelse([$#],[3],[],[m4_define([$1],[$4])$2[]$0([$1],[$2],m4_shift(m4_shift(m4_shift($@))))])])m4_dnl
m4_define(_YEAR_,m4_esyscmd(date +%Y|tr -d '\n'))
<!DOCTYPE html>
<!-- saved from url=(0054)http://leafletjs.com/examples/quick-start-example.html -->
m4_ifelse(IOSAPP,[true],
<!-- Related to issue #5841: the iOS app sets the base text direction via the "dir" parameter -->
<html dir="" style="height:100%"><head><meta http-equiv="Content-Type" content="text/html; charset=UTF-8" data-theme="%UI_THEME%">
,
<html %UI_RTL_SETTINGS% style="height:100%"><head><meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
)m4_dnl
<title>Online Editor</title>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0 minimum-scale=1.0, maximum-scale=1.0, user-scalable=no">
<meta name="previewImg" content="data:image/svg+xml;base64,PHN2ZyB4bWxucz0naHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmcnIHZpZXdCb3g9JzAgMCAyMDAgMjAwJz4KICAgPGNpcmNsZSB0cmFuc2Zvcm09J3JvdGF0ZSgwKScgdHJhbnNmb3JtLW9yaWdpbj0nY2VudGVyJyBmaWxsPSdub25lJyBzdHJva2U9JyNCNkI2QjYnIHN0cm9rZS13aWR0aD0nMTUnIHN0cm9rZS1saW5lY2FwPSdyb3VuZCcgc3Ryb2tlLWRhc2hhcnJheT0nMjMwIDEwMDAnIHN0cm9rZS1kYXNob2Zmc2V0PScwJyBjeD0nMTAwJyBjeT0nMTAwJyByPSc3MCc+CiAgICAgPGFuaW1hdGVUcmFuc2Zvcm0KICAgICAgICAgYXR0cmlidXRlTmFtZT0ndHJhbnNmb3JtJwogICAgICAgICB0eXBlPSdyb3RhdGUnCiAgICAgICAgIGZyb209JzAnCiAgICAgICAgIHRvPSczNjAnCiAgICAgICAgIGR1cj0nMicKICAgICAgICAgcmVwZWF0Q291bnQ9J2luZGVmaW5pdGUnPgogICAgICA8L2FuaW1hdGVUcmFuc2Zvcm0+CiAgIDwvY2lyY2xlPgo8L3N2Zz4=">
<meta name="previewSmile" content="data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIGhlaWdodD0iMjRweCIgdmlld0JveD0iMCAtOTYwIDk2MCA5NjAiIHdpZHRoPSIyNHB4IiBmaWxsPSIjNWY2MzY4Ij48cGF0aCBkPSJtNDI0LTI5NiAyODItMjgyLTU2LTU2LTIyNiAyMjYtMTE0LTExNC01NiA1NiAxNzAgMTcwWm01NiAyMTZxLTgzIDAtMTU2LTMxLjVUMTk3LTE5N3EtNTQtNTQtODUuNS0xMjdUODAtNDgwcTAtODMgMzEuNS0xNTZUMTk3LTc2M3E1NC01NCAxMjctODUuNVQ0ODAtODgwcTgzIDAgMTU2IDMxLjVUNzYzLTc2M3E1NCA1NCA4NS41IDEyN1Q4ODAtNDgwcTAgODMtMzEuNSAxNTZUNzYzLTE5N3EtNTQgNTQtMTI3IDg1LjVUNDgwLTgwWm0wLTgwcTEzNCAwIDIyNy05M3Q5My0yMjdxMC0xMzQtOTMtMjI3dC0yMjctOTNxLTEzNCAwLTIyNyA5M3QtOTMgMjI3cTAgMTM0IDkzIDIyN3QyMjcgOTNabTAtMzIwWiIvPjwvc3ZnPg==">

<script>
m4_dnl# Define MOBILEAPP as true if this is either for the iOS app or for the gtk+ "app" testbed
m4_define([MOBILEAPP],[])
m4_ifelse(IOSAPP,[true],[m4_define([MOBILEAPP],[true])])
m4_ifelse(GTKAPP,[true],[m4_define([MOBILEAPP],[true])])
m4_ifelse(ANDROIDAPP,[true],[m4_define([MOBILEAPP],[true])])

// FIXME: This is temporary and not what we actually eventually want.

// What we really want is not a separate HTML file (produced with M4 conditionals on the below
// EMSCRIPTENAPP) for a "WASM app". What we want is that the same lool.html page adapts on demand to
// instead run locally using WASM, if the connection to the LOOL server breaks. (And then
// re-connects to the LOOL server when possible.)

m4_ifelse(EMSCRIPTENAPP,[true],[m4_define([MOBILEAPP],[true])])

m4_ifelse(MOBILEAPP,[],
  window.welcomeUrl = '%WELCOME_URL%';
  window.feedbackUrl = '%FEEDBACK_URL%';

  // Start listening for Host_PostmessageReady message and save the
  // result for future
  window.WOPIpostMessageReady = false;
  var PostMessageReadyListener = function(e) {
    if (!(e && e.data))
        return;

    try {
        var msg = JSON.parse(e.data);
    } catch (err) {
        return;
    }

    if (msg.MessageId === 'Host_PostmessageReady') {
      window.WOPIPostmessageReady = true;
      window.removeEventListener('message', PostMessageReadyListener, false);
      console.log('Received Host_PostmessageReady.');
    }
  };
  window.addEventListener('message', PostMessageReadyListener, false);
)m4_dnl

m4_dnl# For use in conditionals in JS: window.ThisIsAMobileApp, window.ThisIsTheiOSApp,
m4_dnl# and window.ThisIsTheGtkApp

m4_ifelse(MOBILEAPP,[true],
  [   window.ThisIsAMobileApp = true;
   // Fix issue #5841 by setting the welcome, feedback, and buy product URLs
   // to empty for mobile
   window.welcomeUrl = '';
   window.feedbackUrl = '';
   window.HelpFile = String.raw`m4_syscmd([cat html/lool-help.html])`;
   window.open = function (url, windowName, windowFeatures) {
     window.postMobileMessage('HYPERLINK ' + url); /* don't call the 'normal' window.open on mobile at all */
   }
   window.MobileAppName='MOBILEAPPNAME';
   brandProductName='MOBILEAPPNAME';],
  [   window.ThisIsAMobileApp = false;]
)
m4_ifelse(IOSAPP,[true],
  [   window.ThisIsTheiOSApp = true;
   window.postMobileMessage = function(msg) { window.webkit.messageHandlers.lok.postMessage(msg); };
   window.postMobileError   = function(msg) { window.webkit.messageHandlers.error.postMessage(msg); };
   window.postMobileDebug   = function(msg) { window.webkit.messageHandlers.debug.postMessage(msg); };],
  [   window.ThisIsTheiOSApp = false;]
)
m4_ifelse(GTKAPP,[true],
  [   window.ThisIsTheGtkApp = true;
   window.postMobileMessage = function(msg) { window.webkit.messageHandlers.lok.postMessage(msg, '*'); };
   window.postMobileError   = function(msg) { window.webkit.messageHandlers.error.postMessage(msg, '*'); };
   window.postMobileDebug   = function(msg) { window.webkit.messageHandlers.debug.postMessage(msg, '*'); };],
  [   window.ThisIsTheGtkApp = false;]
)
m4_ifelse(ANDROIDAPP,[true],
  [   window.ThisIsTheAndroidApp = true;
   window.postMobileMessage = function(msg) { window.LOOLMessageHandler.postMobileMessage(msg); };
   window.postMobileError   = function(msg) { window.LOOLMessageHandler.postMobileError(msg); };
   window.postMobileDebug   = function(msg) { window.LOOLMessageHandler.postMobileDebug(msg); };],
  [   window.ThisIsTheAndroidApp = false;]
)
m4_ifelse(EMSCRIPTENAPP,[true],
  [   window.ThisIsTheEmscriptenApp = true;
   window.postMobileMessage = function(msg) { app.HandleLOOLMessage(app.AllocateUTF8(msg)); };
   window.postMobileError   = function(msg) { console.log('LOOL Error: ' + msg); };
   window.postMobileDebug   = function(msg) { console.log('LOOL Debug: ' + msg); };],
  [   window.ThisIsTheEmscriptenApp = false;]
)

// This function may look unused, but it's needed in WASM and Android to send data through the fake websocket. Please
// don't remove it without first grepping for 'Base64ToArrayBuffer' in the C++ code
var Base64ToArrayBuffer = function(base64Str) {
  var binStr = atob(base64Str);
  var ab = new ArrayBuffer(binStr.length);
  var bv = new Uint8Array(ab);
  for (var i = 0, l = binStr.length; i < l; i++) {
    bv[[i]] = binStr.charCodeAt(i);
  }
  return ab;
}

  window.bundlejsLoaded = false;
  window.fullyLoadedAndReady = false;
  window.addEventListener('load', function() {
    window.fullyLoadedAndReady = true;
  }, false);

</script>

m4_ifelse(EMSCRIPTENAPP,[true],[
  <script>
    console.log('================ Before including online.js');
  </script>
  <script type="text/javascript" src="online.js"></script>
  <script>
    console.log('================ After including online.js');
  </script>
])

m4_ifelse(BUNDLE,[],
  <!-- Using individual CSS files -->
  m4_foreachq([fileCSS],[LOOL_CSS],[<link rel="stylesheet" href="][m4_ifelse(MOBILEAPP,[],[%SERVICE_ROOT%/browser/%VERSION%/])][fileCSS" />
]),
  [<!-- Dynamically load the bundle.css -->
<script>
var link = document.createElement('link');
link.setAttribute("rel", "stylesheet");
link.setAttribute("type", "text/css");
link.setAttribute("href", '][m4_ifelse(MOBILEAPP,[],[%SERVICE_ROOT%/browser/%VERSION%/])][bundle.css');
document.getElementsByTagName("head")[[0]].appendChild(link);
</script>
])
<!--%BRANDING_CSS%--> <!-- add your logo here -->
m4_ifelse(IOSAPP,[true],
  [<link rel="stylesheet" href="Branding/branding.css">])
m4_ifelse(ANDROIDAPP,[true],
  [<link rel="stylesheet" href="branding.css">])
m4_ifelse(EMSCRIPTENAPP,[true],
  [<link rel="stylesheet" href="branding.css">])

m4_dnl Handle localization
m4_ifelse(MOBILEAPP,[true],
  [
   m4_ifelse(IOSAPP,[true],
     [],
     [<link rel="localizations" href="l10n/uno-localizations-override.json" type="application/vnd.oftn.l10n+json"/>
      <link rel="localizations" href="l10n/localizations.json" type="application/vnd.oftn.l10n+json"/>
      <link rel="localizations" href="l10n/locore-localizations.json" type="application/vnd.oftn.l10n+json"/>
      <link rel="localizations" href="l10n/help-localizations.json" type="application/vnd.oftn.l10n+json"/>
      <link rel="localizations" href="l10n/uno-localizations.json" type="application/vnd.oftn.l10n+json"/>])],
  [<link rel="localizations" href="%SERVICE_ROOT%/browser/%VERSION%/l10n/uno-localizations-override.json" type="application/vnd.oftn.l10n+json"/>
   <link rel="localizations" href="%SERVICE_ROOT%/browser/%VERSION%/l10n/localizations.json" type="application/vnd.oftn.l10n+json"/>
   <link rel="localizations" href="%SERVICE_ROOT%/browser/%VERSION%/l10n/locore-localizations.json" type="application/vnd.oftn.l10n+json"/>
   <link rel="localizations" href="%SERVICE_ROOT%/browser/%VERSION%/l10n/help-localizations.json" type="application/vnd.oftn.l10n+json"/>
   <link rel="localizations" href="%SERVICE_ROOT%/browser/%VERSION%/l10n/uno-localizations.json" type="application/vnd.oftn.l10n+json"/>]
)m4_dnl
</head>

  <body style="user-select: none;height:100%;display:flex;flex-direction:column">
    <!--The "controls" div holds map controls such as the Zoom button and
        it's separated from the map in order to have the controls on the top
        of the page all the time.

        The "document-container" div is the actual display of the document, is
        what the user sees and it should be no larger than the screen size.

        The "map" div is the actual document and it has the document's size
        and width, this being inside the smaller "document-container" will
        cause the content to overflow, creating scrollbars -->

    <nav class="main-nav" role="navigation">
      <!-- Mobile menu toggle button (hamburger/x icon) -->
      <input id="main-menu-state" type="checkbox" style="display: none"/>
      <ul id="main-menu" class="sm sm-simple lo-menu readonly"></ul>
      <div id="document-titlebar">
        <div class="document-title">
          <!-- visuallyhidden: hide it visually but keep it available to screen reader and other assistive technology -->
          <label class="visuallyhidden" for="document-name-input" aria-hidden="false">Document name</label>
          <input id="document-name-input" type="text" spellcheck="false" disabled="true" style="display: none"/>
          <div id="document-name-input-loading-bar"></div>
          <progress id="document-name-input-progress-bar" class="progress-bar" value="0" max="99"></progress>
        </div>
      </div>

      <div id="userListHeader">
        <div id="followingChipBackground">
          <div id="followingChip"></div>
        </div>
        <div id="userListSummaryBackground"><button id="userListSummary"></button></div>
      </div>
      <div id="closebuttonwrapperseparator"></div>
      <div id="closebuttonwrapper">
        <div class="closebuttonimage" id="closebutton"></div>
      </div>
     </nav>

     <div id="toolbar-wrapper" role="toolbar" aria-orientation="horizontal">
        <div id="toolbar-row" class="toolbar-row">
          <div id="toolbar-logo"></div>
          <div id="toolbar-mobile-back" class="editmode-off"></div>
          <div class="jsdialog ui-spacer"></div>
          <div id="toolbar-up"></div>
          <div id="toolbar-hamburger">
            <label class="main-menu-btn" for="main-menu-state">
              <span class="main-menu-btn-icon" id="main-menu-btn-icon"></span>
            </label>
          </div>
        </div>
       <div colspan="4" id="formulabar" style="display: none"></div>
       <progress id="mobile-progress-bar" class="progress-bar" value="0" max="99"></progress>
    </div>

    <input id="insertgraphic" aria-labelledby="menu-insertgraphic" type="file" accept="image/*" style="position: fixed; top: -100em">
    <input id="selectbackground" aria-labelledby="menu-selectbackground" type="file" accept="image/*" style="position: fixed; top: -100em">

    <div id="main-document-content" style="display:flex; flex-direction: row; flex: 1; margin: 0; padding: 0; min-height: 0">
      <div id="presentation-controls-wrapper" class="readonly">
        <div id="slide-sorter"></div>
        <div id="presentation-toolbar" style="display: none"></div>
      </div>
      <div id="document-container" class="readonly" dir="ltr">
        <div id="map"></div>
      </div>
      <div id="sidebar-dock-wrapper" style="display: none;">
        <div id="sidebar-panel"></div>
      </div>
    </div>

    <div id="spreadsheet-toolbar" style="display: none"></div>

    <div id="mobile-edit-button" style="display: none">
      <div id="mobile-edit-button-image"></div>
    </div>

    <div id="toolbar-down" style="display: none"></div>
    <div id="toolbar-search" style="display: none"></div>
    <div id="mobile-wizard" style="display: none">
      <div id="mobile-wizard-tabs"></div>
      <table id="mobile-wizard-titlebar" class="mobile-wizard-titlebar" width="100%">
        <tr>
          <td id="mobile-wizard-back" class="mobile-wizard-back"></td>
          <td id="mobile-wizard-title" class="mobile-wizard-title ui-widget"></td>
        </tr>
      </table>
      <div id="mobile-wizard-content"></div>
    </div>

    <!-- Remove if you don't want the About dialog -->
    <div id="about-dialog" style="display:none; user-select: text" tabIndex="0">
      <div id="about-dialog-header">
        <fig id="integrator-logo"></fig>
        <h1 id="product-name">LibreOffice Online</h1>
      </div>
      <hr/>
      <div id="about-dialog-container">
        <div id="about-dialog-logos">
          <fig id="product-logo"></fig>
          <fig id="lokit-logo"></fig>
        </div>
        <div id="about-dialog-info-container">
          <div id="about-dialog-info">
            <div id="loolwsd-version-label"></div>
            <div style="margin-inline-end: auto;"><div id="loolwsd-version" dir="ltr"></div></div>
            <div class="spacer"></div>
            <div id="lokit-version-label"></div>
            <div style="margin-inline-end: auto;"><div id="lokit-version" dir="ltr"></div></div>
            m4_ifelse(MOBILEAPP,[],[<div id="served-by"><span id="served-by-label"></span>&nbsp;<span id="os-info"></span>&nbsp;<wbr><span id="loolwsd-id"></span></div>],[<p></p>])
            <div id="slow-proxy"></div>
            m4_ifelse(DEBUG,[true],[<div id="js-dialog">JSDialogs: <a href="javascript:void(function() { app.socket.sendMessage('uno .uno:WidgetTestDialog') }() )">View widgets</a></div>])
            <div id="routeToken"></div>
            <div id="wopi-host-id" style="display: none">%WOPI_HOST_ID%</div>
            <div id="proxy-prefix-id" style="display: none">%PROXY_PREFIX_ENABLED%</div>
            <p style="margin-inline-end: auto;"><span dir="ltr">Copyright © _YEAR_, VENDOR.</span></p>
          </div>
        </div>
      </div>
    </div>

    <script>
m4_ifelse(MOBILEAPP,[true],
     [window.host = '';
      window.serviceRoot = '';
      window.hexifyUrl = false;
      // We can't use %VERSION% here as there is no FileServer.cpp involved in a mobile app that
      // would expand the %FOO% things. But it seems that window.versionPath is not used in the
      // mobile apps anyway.
      // window.versionPath = 'UNKNOWN';
      m4_ifelse(EMSCRIPTENAPP,[true],
              [window.accessToken = '%ACCESS_TOKEN%';
              window.accessTokenTTL = '%ACCESS_TOKEN_TTL%';
              window.accessHeader = '%ACCESS_HEADER%';],
              [window.accessToken = '';
              window.accessTokenTTL = '';
              window.accessHeader = '';]
      )
      window.postMessageOriginExt = '';
      window.loolLogging = 'true';
      window.enableWelcomeMessage = false;
      window.autoShowWelcome = false;
      window.autoShowFeedback = true;
      window.allowUpdateNotification = false;
      window.outOfFocusTimeoutSecs = 1000000;
      window.idleTimeoutSecs = 1000000;
      window.protocolDebug = false;
      window.enableDebug = false;
      window.frameAncestors = '';
      window.socketProxy = false;
      window.tileSize = 256;
      window.uiDefaults = {};
      window.useIntegrationTheme = 'false';
      window.checkFileInfoOverride = {};
      window.deeplEnabled = false;
      window.zoteroEnabled = false;
      window.savedUIState = true;
      window.wasmEnabled = false;
      window.indirectionUrl='';],
     [window.host = '%HOST%';
      window.serviceRoot = '%SERVICE_ROOT%';
      window.hexifyUrl = %HEXIFY_URL%;
      window.versionPath = '%VERSION%';
      window.accessToken = '%ACCESS_TOKEN%';
      window.accessTokenTTL = '%ACCESS_TOKEN_TTL%';
      window.accessHeader = '%ACCESS_HEADER%';
      window.postMessageOriginExt = '%POSTMESSAGE_ORIGIN%';
      window.loolLogging = '%BROWSER_LOGGING%';
      window.loolwsdVersion = '%LOOLWSD_VERSION%';
      window.enableWelcomeMessage = %ENABLE_WELCOME_MSG%;
      window.autoShowWelcome = %AUTO_SHOW_WELCOME%;
      window.autoShowFeedback = %AUTO_SHOW_FEEDBACK%;
      window.allowUpdateNotification = %ENABLE_UPDATE_NOTIFICATION%;
      window.userInterfaceMode = '%USER_INTERFACE_MODE%';
      window.useIntegrationTheme = '%USE_INTEGRATION_THEME%';
      window.enableMacrosExecution = '%ENABLE_MACROS_EXECUTION%';
      window.enableAccessibility = '%ENABLE_ACCESSIBILITY%' === 'true';
      window.outOfFocusTimeoutSecs = %OUT_OF_FOCUS_TIMEOUT_SECS%;
      window.idleTimeoutSecs = %IDLE_TIMEOUT_SECS%;
      window.protocolDebug = %PROTOCOL_DEBUG%;
      window.enableDebug = %ENABLE_DEBUG%;
      window.frameAncestors = decodeURIComponent('%FRAME_ANCESTORS%');
      window.socketProxy = %SOCKET_PROXY%;
      window.tileSize = 256;
      window.uiDefaults = %UI_DEFAULTS%;
      window.checkFileInfoOverride = %CHECK_FILE_INFO_OVERRIDE%;
      window.deeplEnabled = %DEEPL_ENABLED%;
      window.zoteroEnabled = %ZOTERO_ENABLED%;
      window.savedUIState = %SAVED_UI_STATE%;
      window.wasmEnabled = %WASM_ENABLED%;
      window.indirectionUrl='%INDIRECTION_URL%';])

// This is GLOBAL_JS:
m4_syscmd([cat ]GLOBAL_JS)m4_dnl

// Related to issue #5841: the iOS app sets the base text direction via the
// "dir" parameter
m4_ifelse(IOSAPP,[true],
     [document.dir = window.Params.get('dir');])

m4_ifelse(IOSAPP,[true],
     [window.userInterfaceMode = window.loolParams.get('userinterfacemode');])

m4_ifelse(ANDROIDAPP,[true],
     [window.userInterfaceMode = window.loolParams.get('userinterfacemode');])

var darkTheme = window.loolParams.get('darkTheme');
if (darkTheme) {window.uiDefaults = {'darkTheme': true};}

m4_ifelse(EMSCRIPTENAPP,[true],
     [window.userInterfaceMode = 'notebookbar';])

// Dynamically load the appropriate *-mobile.css, *-tablet.css or *-desktop.css
var link = document.createElement('link');
link.setAttribute("rel", "stylesheet");
link.setAttribute("type", "text/css");
var brandingLink = document.createElement('link');
brandingLink.setAttribute("rel", "stylesheet");
brandingLink.setAttribute("type", "text/css");

var theme_name = '%BRANDING_THEME%';
var theme_prefix = '';lool
if(window.useIntegrationTheme === 'true' && theme_name !== '') {
    theme_prefix = theme_name + '/';
}

if (window.mode.isMobile()) {
    [link.setAttribute("href", ']m4_ifelse(MOBILEAPP,[],[%SERVICE_ROOT%/browser/%VERSION%/])[device-mobile.css');]
    [brandingLink.setAttribute("href", ']m4_ifelse(MOBILEAPP,[],[%SERVICE_ROOT%/browser/%VERSION%/])m4_ifelse(IOSAPP,[true],[Branding/])[' + theme_prefix + 'branding-mobile.css');]
} else if (window.mode.isTablet()) {
    [link.setAttribute("href", ']m4_ifelse(MOBILEAPP,[],[%SERVICE_ROOT%/browser/%VERSION%/])[device-tablet.css');]
    [brandingLink.setAttribute("href", ']m4_ifelse(MOBILEAPP,[],[%SERVICE_ROOT%/browser/%VERSION%/])m4_ifelse(IOSAPP,[true],[Branding/])[' + theme_prefix + 'branding-tablet.css');]
} else {
    [link.setAttribute("href", ']m4_ifelse(MOBILEAPP,[],[%SERVICE_ROOT%/browser/%VERSION%/])[device-desktop.css');]
    [brandingLink.setAttribute("href", ']m4_ifelse(MOBILEAPP,[],[%SERVICE_ROOT%/browser/%VERSION%/])[' + theme_prefix + 'branding-desktop.css');]
}
document.getElementsByTagName("head")[[0]].appendChild(link);
document.getElementsByTagName("head")[[0]].appendChild(brandingLink);
</script>

m4_ifelse(MOBILEAPP,[true],
  <!-- This is for a mobile app so the script files are in the same folder -->
  m4_ifelse(BUNDLE,[],m4_foreachq([fileJS],[LOOL_JS],
  [    <script src="fileJS" defer></script>
  ]),
  [    <script src="bundle.js" defer></script>
  ]),
  m4_ifelse(BUNDLE,[],
      <!-- Using indivisual JS files -->
      m4_foreachq([fileJS],[LOOL_JS],
      [ <script src="%SERVICE_ROOT%/browser/%VERSION%/fileJS" defer></script>
      ]),
  [
       <!-- Using bundled JS files -->
       <script src="%SERVICE_ROOT%/browser/%VERSION%/bundle.js" defer></script>
  ])
)m4_dnl

    m4_ifelse(MOBILEAPP,[true],
    [<script src="m4_ifelse(IOSAPP,[true],[Branding/])branding.js"></script>],
    [<!--%BRANDING_JS%--> <!-- logo onclick handler -->
    <!--%CSS_VARIABLES%-->])
</body></html>
