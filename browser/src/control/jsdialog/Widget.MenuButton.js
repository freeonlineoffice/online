/* -*- js-indent-level: 8 -*- */
/*
 * JSDialog.MenuButton - button which can trigger some action or open a menu
 *
 * Example JSON:
 * {
 *     id: 'id:MenuId', (where MenuId is custom menu id, menu is stored in the builder)
 *     type: 'menubutton',
 *     text: 'Label',
 *     image: 'base64 encoded icon',
 *     command: '.uno:Command',
 *     enabled: false
 * }
 *
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

/* global JSDialog $ */

function _menubuttonControl (parentContainer, data, builder) {
	var ids;
	var menuId = null;

	if (data.id.includes(':')) {
		ids = data.id.split(':');
		menuId = ids[1];
		data.id = ids[0];
	}
	else if (data.id.includes('-')) {
		ids = data.id.split('-');
		menuId = ids[1];
		data.id = ids[0];
	}
	else
		menuId = data.id;

	// import menu
	if (data.menu) {
		menuId = data.id + '-menu';
		var builtMenu = [];
		for (var i in data.menu) {
			builtMenu.push({
				id: data.menu[i].id,
				text: data.menu[i].text
			});
		}
		builder._menus.set(menuId, builtMenu);
	}

	var menuEntries = menuId ? builder._menus.get(menuId) : null;

	if (menuEntries) {
		var noLabels = builder.options.noLabelsForUnoButtons;
		builder.options.noLabelsForUnoButtons = data.noLabel ? data.noLabel : false;

		// command is needed to generate image
		if (!data.command)
			data.command = menuId;

		var options = {hasDropdownArrow: menuEntries.length > 1};
		var control = builder._unoToolButton(parentContainer, data, builder, options);

		$(control.container).tooltip({disabled: true});
		$(control.container).addClass('menubutton');

		$(control.container).unbind('click');

		var dropdownId = data.id;
		var clickFunction = function () {
			if (control.container.hasAttribute('disabled'))
				return;

			var callback = function(objectType, eventType, object, data, entry) {
				if (eventType === 'selected' && entry.uno) {
					var uno = (entry.uno.indexOf('.uno:') === 0) ? entry.uno : '.uno:' + entry.uno;
					builder.map.sendUnoCommand(uno);
					JSDialog.CloseDropdown(dropdownId);
					return true;
				} else if (eventType === 'selected' && entry.action) {
					builder.map.dispatch(entry.action);
					JSDialog.CloseDropdown(dropdownId);
					return true;
				} else if (eventType === 'selected') {
					builder.callback('menubutton', 'select', control.container, entry.id, builder);
					JSDialog.CloseDropdown(dropdownId);
					return true;
				}

				return false;
			};

			if (menuEntries.length === 1) {
				callback(null, 'selected', null, null, menuEntries[0]);
			} else {
				JSDialog.OpenDropdown(dropdownId, control.container, menuEntries, callback);
			}
		};

		control.container.addEventListener('click', clickFunction);
		builder._preventDocumentLosingFocusOnClick(control.container);

		builder.options.noLabelsForUnoButtons = noLabels;
	} else if (data.text !== undefined || data.image) {
		var button = L.DomUtil.create('button', 'menubutton ' + builder.options.cssClass, parentContainer);
		button.id = data.id;
		button.setAttribute('aria-haspopup', true);
		if (data.image) {
			var image = L.DomUtil.create('img', '', button);
			image.src = data.image;
		}
		var label = L.DomUtil.create('span', '', button);
		label.innerText = data.text ? data.text : '';
		L.DomUtil.create('i', 'arrow', button);

		$(button).click(function () {
			if (!button.hasAttribute('disabled')) {
				builder.callback('menubutton', 'toggle', button, undefined, builder);
			}
		});

		if (data.enabled === false)
			button.setAttribute('disabled', '');
	} else {
		window.app.console.warn('Not found menu "' + menuId + '"');
	}

	return false;
}

JSDialog.menubuttonControl = function (parentContainer, data, builder) {
	var buildInnerData = _menubuttonControl(parentContainer, data, builder);
	return buildInnerData;
};
