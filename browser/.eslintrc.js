module.exports = {
	'env': {
		'browser': true,
		'es2021': true,
        'node': true
	},
	'globals': {
		'L': true,
		'module': false,
		'define': false
	},
	'extends': [
		'eslint:recommended',
		'plugin:@typescript-eslint/recommended',
	],
	'overrides': [
	],
	'parser': '@typescript-eslint/parser',
	'parserOptions': {
		'ecmaVersion': 'latest',
		'sourceType': 'module',
		'allowImportExportEverywhere': true,
	},
	'plugins': [
		'@typescript-eslint'
	],
	'rules': {
		'no-mixed-spaces-and-tabs': [2, 'smart-tabs'],
		'@typescript-eslint/no-unused-vars': 'off',
		'no-empty-function': 'off',
		'@typescript-eslint/no-empty-function': 'off',
		'@typescript-eslint/no-inferrable-types': 'off',
		'no-var': 'off',
		'@typescript-eslint/no-explicit-any': 'off',
		'@typescript-eslint/no-namespace': 'off',
		'no-inner-declarations': 'off',
		'no-constant-condition': 'off',
		'@typescript-eslint/triple-slash-reference': 'off',
		'@typescript-eslint/no-this-alias': 'off',
		'camelcase': 2,
		'quotes': [2, 'single'],
		'space-in-parens': 2,
		'space-before-blocks': 2,
		'keyword-spacing': 2,
		'no-lonely-if': 2,
		'comma-style': 2,
		'indent': [2, 'tab', {'VariableDeclarator': 0}],
		'no-underscore-dangle': 0,
		'no-multi-spaces': 0,
		'strict': 0,
		'key-spacing': 0,
		'no-shadow': 0,
		'no-console': 0,
		'no-control-regex': 0,
		'no-useless-escape': 0,
		'semi': 2,
		'no-redeclare': 0,
	}
};
