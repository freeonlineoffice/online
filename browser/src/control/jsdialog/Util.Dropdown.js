/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

/*
 * Util.Dropdown - helper to create dropdown menus for JSDialogs
 */

/* global JSDialog app */

function _createDropdownId(id) {
	return id + '-dropdown';
}

JSDialog.CreateDropdownEntriesId = function(id) {
	return id + '-entries';
}

JSDialog.OpenDropdown = function (id, popupParent, entries, innerCallback, popupAnchor, isSubmenu) {
	var json = {
		id: _createDropdownId(id),
		type: 'dropdown',
		isSubmenu: isSubmenu,
		jsontype: 'dialog',
		popupParent: popupParent,
		popupAnchor: popupAnchor,
		gridKeyboardNavigation: false,
		cancellable: true,
		children: [
			{
				id: JSDialog.CreateDropdownEntriesId(id),
				type: 'grid',
				allyRole: 'listbox',
				cols: 1,
				rows: entries.length,
				children: []
			}
		]
	};

	if (popupParent && typeof popupParent._onDropDown === 'function') {
		popupParent._onDropDown(true);
	}

	var isChecked = function (unoCommand) {
		var items = L.Map.THIS['stateChangeHandler'];
		var val = items.getItemValue(unoCommand);

		if (val && (val === true || val === 'true'))
			return true;
		else
			return false;
	};

	for (var i in entries) {
		var checkedValue = (entries[i].checked === undefined)
			? undefined : (entries[i].uno && isChecked('.uno' + entries[i].uno));

		var entry;

		if (entries[i].type === 'json') {
			// replace old grid with new widget
			json.children[0] = entries[i].content;
			if (json.children[0].type === 'grid') json.gridKeyboardNavigation = true;
			break;
		}

		switch (entries[i].type) {
			// DEPRECACTED: legacy plain HTML adapter
			case 'html':
				entry = {
					id: id + '-entry-' + i,
					type: 'htmlcontent',
					htmlId: entries[i].htmlId,
					closeCallback: function () { JSDialog.CloseDropdown(id); }
				};
				json.gridKeyboardNavigation = true;
			break;

			// dropdown is a colorpicker
			case 'colorpicker':
				entry = entries[i];
				// for color picker we have a "KeyboardGridNavigation" function defined separately to handle custom cases
				json.gridKeyboardNavigation = true;
			break;

			// menu and submenu entry
			case 'action':
			case 'menu':
			default:
				entry = {
					id: id + '-entry-' + i,
					type: 'comboboxentry',
					customRenderer: entries[i].customRenderer,
					comboboxId: id,
					pos: i,
					text: entries[i].text,
					hint: entries[i].hint,
					w2icon: entries[i].icon, // FIXME: DEPRECATED
					icon: entries[i].img,
					checked: entries[i].checked || checkedValue,
					selected: entries[i].selected,
					hasSubMenu: !!entries[i].items
				};
			break;

			// allows to put regular JSDialog JSON into popup
			case 'json':
				entry = entries[i].content;
				json.gridKeyboardNavigation = true;
			break;

			// horizontal separator in menu
			case 'separator':
				entry = {
					id: id + '-entry-' + i,
					type: 'separator',
					orientation: 'horizontal'
				};
			break;
		}

		json.children[0].children.push(entry);
	}

	var lastSubMenuOpened = null;
	var generateCallback = function (targetEntries) {
		return function(objectType, eventType, object, data) {
			var pos = data ? parseInt(data.substr(0, data.indexOf(';'))) : null;
			var entry = targetEntries && pos !== null ? targetEntries[pos] : null;

			if (eventType === 'selected' || eventType === 'showsubmenu') {
				if (entry.items) {
					if (lastSubMenuOpened) {
						var submenu = JSDialog.GetDropdown(lastSubMenuOpened);
						if (submenu) {
							JSDialog.CloseDropdown(lastSubMenuOpened);
							lastSubMenuOpened = null;
						}
					}

					// open submenu
					var dropdown = JSDialog.GetDropdown(object.id);
					var subMenuId = object.id + '-' + pos;
					var targetEntry = dropdown.querySelectorAll('.ui-grid-cell')[pos + 1];
					JSDialog.OpenDropdown(subMenuId, targetEntry, entry.items,
						generateCallback(entry.items), 'top-end', true);
					lastSubMenuOpened = subMenuId;

					app.layoutingService.appendLayoutingTask(() => {
						var dropdown = JSDialog.GetDropdown(subMenuId);
						if (!dropdown) {
							console.debug('Dropdown: missing :' + subMenuId);
							return;
						}
						var container = dropdown.querySelector('.ui-grid');
						JSDialog.MakeFocusCycle(container);
						var focusables = JSDialog.GetFocusableElements(container);
						if (focusables && focusables.length)
							focusables[0].focus();
					});
				} else if (eventType === 'selected' && entry.uno) {
					var uno = (entry.uno.indexOf('.uno:') === 0) ? entry.uno : '.uno:' + entry.uno;
					L.Map.THIS.sendUnoCommand(uno);
					JSDialog.CloseDropdown(id);
					return;
				}
			} else if (!lastSubMenuOpened && eventType === 'hidedropdown') {
				JSDialog.CloseDropdown(id);
			}

			// for multi-level menus last parameter should be used to handle event (it contains selected entry)
			if (innerCallback && innerCallback(objectType, eventType, object, data, entry))
				return;

			if (eventType === 'selected')
				JSDialog.CloseDropdown(id);
			else
				console.debug('Dropdown: unhandled action: "' + eventType + '"');
		};
	};
	L.Map.THIS.fire('closepopups'); // close popups if a dropdown menu is opened
	L.Map.THIS.fire('jsdialog', {data: json, callback: generateCallback(entries)});
};

JSDialog.CloseDropdown = function (id) {
	L.Map.THIS.fire('jsdialog', {data: {
		id: _createDropdownId(id),
		jsontype: 'dialog',
		action: 'close'
	}});
};

JSDialog.CloseAllDropdowns = function () {
	L.Map.THIS.jsdialog.closeAllDropdowns();
};

JSDialog.GetDropdown = function (id) {
	return document.body.querySelector('#' + _createDropdownId(id));
};
