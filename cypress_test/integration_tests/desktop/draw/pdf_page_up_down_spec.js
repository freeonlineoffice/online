/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'PDF View Tests', function() {
	var newFilePath;

	beforeEach(function() {
		newFilePath = helper.setupAndLoadDocument('draw/pdf_page_up_down.pdf');
	});

	it('PDF page down', { env: { 'pdf-view': true } }, function() {
		cy.cGet('#map').type('{pagedown}');
		cy.cGet('#map').type('{pagedown}');
		cy.cGet('#preview-frame-part-1 img').should('have.class', 'preview-img-currentpart');
		cy.cGet('#map').type('{pageup}');
		cy.cGet('#map').type('{pageup}');
		cy.cGet('#preview-frame-part-0 img').should('have.class', 'preview-img-currentpart');
	});

	it.skip('PDF insert comment', { env: { 'pdf-view': true }, defaultCommandTimeout: 60000 }, function() {

		// Insert some comment into the PDF.
		desktopHelper.insertComment();
		cy.cGet('.lool-annotation-content-wrapper').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text0');

		// Close to test save. PDFs cannot really be edited,
		// only comments can be inserted, so they are not saved
		// directly, rather save-as is used. This failed because
		// DocBroker expected to get ModifiedStatus=false, which
		// never happens with save-as and so we couldn't unload.
		helper.closeDocument(newFilePath);

		// TODO: verify comment still exists
		// helper.reloadDocument(newFilePath);
	});
});
