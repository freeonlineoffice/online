/* global describe it cy require beforeEach Cypress */

var desktopHelper = require('../../common/desktop_helper');
var helper = require('../../common/helper');
var { addSlide, changeSlide } = require('../../common/impress_helper');

describe(['tagdesktop'], 'Annotation Tests', function() {

	beforeEach(function() {
		// Give more horizontal room so that comments do not fall off the right
		// side of the screen, causing scrolling or hidden buttons
		cy.viewport(1500, 600);
		helper.setupAndLoadDocument('impress/comment_switching.odp');
		desktopHelper.switchUIToNotebookbar();

		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.hideSidebar();
		}

		cy.cGet('#options-modify-page').click();
		desktopHelper.selectZoomLevel('50', false);
	});

	it('Insert', function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
	});

	it('Insert into the second slide.', function() {
		addSlide(1);
		cy.cGet('#SlideStatus').should('contain','Slide 2 of 2');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
	});

	it('Modify', function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('[id^=annotation-content-area-]').should('contain','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Modify').click();
		cy.cGet('[id^=annotation-modify-textarea-]').type(', some other text');
		cy.cGet('[id^=annotation-save-]').click();
		cy.cGet('[id^=annotation-content-area-]').should('contain','some text0, some other text');
		cy.cGet('.annotation-marker').should('be.visible');
	});

	it('Remove',function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Remove').click();
		cy.cGet('.annotation-marker').should('not.exist');
	});

	it('Reply',function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Reply').click();
		cy.cGet('[id^=annotation-reply-textarea-]').type('some reply text');
		cy.cGet('[id^=annotation-reply-].button-primary').click();
		cy.cGet('.lool-annotation-content > div').should('include.text','some reply text');
	});

	it('Tab Navigation', function() {
		desktopHelper.insertComment(undefined, false);

		cy.cGet('.lool-annotation-autosavelabel').should('be.not.visible');
		cy.realPress('Tab');
		cy.cGet('.lool-annotation-autosavelabel').should('be.not.visible');
		cy.cGet('#annotation-cancel-new:focus-visible');

		cy.realPress('Tab');
		cy.cGet('#annotation-save-new:focus-visible');
		cy.cGet('.lool-annotation-autosavelabel').should('be.not.visible');

		cy.realPress('Tab');
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
	});
});

describe(['tagdesktop'], 'Collapsed Annotation Tests', function() {
	var newFilePath;

	beforeEach(function() {
		newFilePath = helper.setupAndLoadDocument('impress/comment_switching.odp');
		desktopHelper.switchUIToNotebookbar();

		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.hideSidebar();
		}

		// TODO: skip sidebar detection on reload
		// cy.cGet('#options-modify-page').click();

		desktopHelper.selectZoomLevel('50', false);
	});

	it('Insert', function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
	});

	it('Modify', function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('[id^=annotation-content-area-]').should('contain','some text0');
		cy.cGet('.lool-annotation-table .avatar-img').click();
		cy.cGet('.lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Modify').click();
		cy.cGet('[id^=annotation-modify-textarea-]').type(', some other text');
		cy.cGet('[id^=annotation-save-]').click();
		cy.cGet('[id^=annotation-content-area-]').should('contain','some text0, some other text');
		cy.cGet('.annotation-marker').should('be.visible');
	});

	it('Remove',function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
		cy.cGet('.lool-annotation-table .avatar-img').click();
		cy.cGet('.lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Remove').click();
		cy.cGet('.annotation-marker').should('not.exist');
	});

	it('Reply',function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
		cy.cGet('.lool-annotation-table .avatar-img').click();
		cy.cGet('.lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Reply').click();
		cy.cGet('[id^=annotation-reply-textarea-]').type('some reply text');
		cy.cGet('[id^=annotation-reply-].button-primary').click();
		cy.cGet('.lool-annotation-content > div').should('include.text','some reply text');
	});

	it('Autosave Collapse', function() {
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		helper.typeIntoDocument('{home}');
		cy.cGet('.lool-annotation-info-collapsed').should('have.text','!');
		cy.cGet('.lool-annotation-info-collapsed').should('be.visible');
		cy.cGet('.lool-annotation-img').click();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('[id^=annotation-save-]').click();
		cy.cGet('.lool-annotation-img').click();
		cy.cGet('[id^=annotation-content-area-]').should('have.text','some text0');
		cy.cGet('.lool-annotation-autosavelabel').should('be.not.visible');
		cy.cGet('.lool-annotation-info-collapsed').should('not.have.text','!');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-info-collapsed').should('be.not.visible');

		helper.reloadDocument(newFilePath);
		cy.cGet('.lool-annotation-img').click();
		cy.cGet('.lool-annotation-lontent-wrapper').should('exist');
		cy.cGet('[id^=annotation-content-area-]').should('have.text','some text0');
		cy.cGet('.lool-annotation-info-collapsed').should('be.not.visible');
	})
});

describe(['tagdesktop'], 'Comment Scrolling',function() {

	beforeEach(function() {
		cy.viewport(1500, 600);
		helper.setupAndLoadDocument('impress/comment_switching.odp');
		desktopHelper.switchUIToNotebookbar();

		cy.cGet('#options-modify-page').click();
		desktopHelper.selectZoomLevel('50', false);
	});

	it('no comment or one comment', function() {
		cy.cGet('.leaflet-control-scroll-down').should('not.exist');
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
	});

	it.skip('omit slides without comments', function() {
		//scroll up
		desktopHelper.insertComment();
		addSlide(2);
		desktopHelper.insertComment();
		cy.cGet('.leaflet-control-scroll-up').should('be.visible');
		cy.cGet('.leaflet-control-scroll-up').click().wait(300);
		cy.cGet('#SlideStatus').should('contain','Slide 1 of 3');

		//scroll down
		cy.cGet('.leaflet-control-scroll-down').should('exist');
		cy.cGet('.leaflet-control-scroll-down').click().wait(1000);
		cy.cGet('#SlideStatus').should('contain','Slide 3 of 3');
	});

	it('switch to previous or next slide',function() {
		addSlide(1);
		desktopHelper.insertComment();
		desktopHelper.insertComment();

		//scroll up
		addSlide(1);
		cy.cGet('.leaflet-control-scroll-up').should('exist');
		cy.cGet('.leaflet-control-scroll-up').click().wait(300);
		cy.cGet('#SlideStatus').should('contain','Slide 2 of 3');

		//scroll down
		changeSlide(1,'previous');
		cy.cGet('.leaflet-control-scroll-down').should('exist');
		cy.cGet('.leaflet-control-scroll-down').click().wait(300);
		cy.cGet('#SlideStatus').should('contain','Slide 2 of 3');
	});
});

describe(['tagdesktop'], 'Annotation Autosave Tests', function() {
	var newFilePath;

	beforeEach(function() {
		cy.viewport(2400, 600);
		newFilePath = helper.setupAndLoadDocument('impress/comment_switching.odp');
		desktopHelper.switchUIToNotebookbar();

		// TODO: skip sidebar detection on reload
		// if (Cypress.env('INTEGRATION') === 'nextcloud') {
			// desktopHelper.hideSidebar();
		// }
		// cy.cGet('#options-modify-page').click();

		desktopHelper.selectZoomLevel('50', false);
	});

	it('Insert autosave', function() {
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.visible');

		helper.reloadDocument(newFilePath);
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');
	});

	it('Insert autosave save', function() {
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('[id^=annotation-save-]').click();
		cy.cGet('.lool-annotation-autosavelabel').should('be.not.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.not.visible');
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');

		helper.reloadDocument(newFilePath);
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');
	});

	it('Insert autosave cancel', function() {
		desktopHelper.insertComment(undefined, false);
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('.modify-annotation [id^=annotation-cancel-]').click();
		cy.cGet('.lool-annotation-autosavelabel').should('not.exist');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('not.exist');
		cy.cGet('.annotation-marker').should('not.exist');
		cy.cGet('.lool-annotation-content > div').should('not.exist');

		helper.reloadDocument(newFilePath);
		cy.cGet('.annotation-marker').should('not.exist');
		cy.cGet('.lool-annotation-content > div').should('not.exist');
	});

	it('Modify autosave', function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('[id^=annotation-content-area-]').should('have.text','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Modify').click();
		cy.cGet('[id^=annotation-modify-textarea-]').type(', some other text');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.visible');

		helper.reloadDocument(newFilePath);
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0, some other text');
	});

	it('Modify autosave save', function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('[id^=annotation-content-area-]').should('have.text','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Modify').click();
		cy.cGet('[id^=annotation-modify-textarea-]').type(', some other text');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('[id^=annotation-save-]').click();
		cy.cGet('[id^=annotation-content-area-]').should('have.text','some text0, some other text');
		cy.cGet('.annotation-marker').should('be.visible');

		helper.reloadDocument(newFilePath);
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0, some other text');
	});

	it('Modify autosave cancel', function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('[id^=annotation-content-area-]').should('have.text','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Modify').click();
		cy.cGet('[id^=annotation-modify-textarea-]').type(', some other text');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('.modify-annotation [id^=annotation-cancel-]').click();
		cy.cGet('[id^=annotation-content-area-]').should('have.text','some text0');
		cy.cGet('.annotation-marker').should('be.visible');

		helper.reloadDocument(newFilePath);
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');
	});

	it('Reply autosave',function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Reply').click();
		cy.cGet('[id^=annotation-reply-textarea-]').type('some reply text');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('[id^=annotation-modify-textarea-]').should('be.visible');
		cy.cGet('[id^=annotation-modify-textarea-]').should('include.text', 'some text0');
		cy.cGet('[id^=annotation-modify-textarea-]').should('include.text', 'some reply text');

		helper.reloadDocument(newFilePath);
		cy.cGet('.lool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.lool-annotation-content > div').should('include.text','some reply text');
	});

	it('Reply autosave save',function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Reply').click();
		cy.cGet('[id^=annotation-reply-textarea-]').type('some reply text');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('[id^=annotation-modify-textarea-]').should('be.visible');
		cy.cGet('[id^=annotation-modify-textarea-]').should('include.text', 'some text0');
		cy.cGet('[id^=annotation-modify-textarea-]').should('include.text', 'some reply text');
		cy.cGet('[id^=annotation-save-]').click();
		cy.cGet('.lool-annotation-autosavelabel').should('be.not.visible');
		cy.cGet('.lool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.lool-annotation-content > div').should('include.text','some text0');
		cy.cGet('.lool-annotation-content > div').should('include.text','some reply text');

		helper.reloadDocument(newFilePath);
		cy.cGet('.lool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.lool-annotation-content > div').should('include.text','some reply text');
	});

	it('Reply autosave cancel',function() {
		desktopHelper.insertComment();
		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Reply').click();
		cy.cGet('[id^=annotation-reply-textarea-]').type('some reply text');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('[id^=annotation-modify-textarea-]').should('be.visible');
		cy.cGet('[id^=annotation-modify-textarea-]').should('include.text', 'some text0');
		cy.cGet('[id^=annotation-modify-textarea-]').should('include.text', 'some reply text');
		cy.cGet('.modify-annotation [id^=annotation-cancel-]').click();
		cy.cGet('.lool-annotation-autosavelabel').should('be.not.visible');
		cy.cGet('.lool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');

		helper.reloadDocument(newFilePath);
		cy.cGet('.lool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');
	});
});
