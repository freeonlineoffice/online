/* -*- js-indent-level: 8 -*- */
/*
 * L.Control.Selection enables by mouse drag selection in viewing mode
 */

/* global app */

L.Control.Selection = L.Control.extend({
	options: {
		position: 'topleft'
	},

	onAdd: function () {
		var partName = 'leaflet-control-editviewswitch',
		    container = L.DomUtil.create('label', partName + ' leaflet-bar');

		this._checkBox = L.DomUtil.create('input', 'editview-cb', container);
		this._checkBox.type = 'checkbox';
		L.DomEvent.on(this._checkBox, 'change', this._onChange, this);
		app.events.on('updatepermission', this._onUpdatePermission.bind(this));
		container.appendChild(document.createTextNode('Enable Selection'));
		return container;
	},

	_onChange: function () {
		this._map.focus();
	},

	_onUpdatePermission: function (e) {
		if (e.detail.perm === 'edit') {
			this._checkBox.checked = false;
			this._checkBox.disabled = true;
		}
		else if (e.detail.perm === 'view') {
			this._checkBox.checked = false;
			this._checkBox.disabled = false;
		}
		else if (e.detail.perm === 'readonly') {
			this._checkBox.checked = false;
			this._checkBox.disabled = false;
		}
	}
});

L.control.selection = function (options) {
	return new L.Control.Selection(options);
};
