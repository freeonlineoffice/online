module.exports = {
    "rules": {
        "no-mixed-spaces-and-tabs": [2, "smart-tabs"],
    },
    "env": {
        "browser": true,
        "es2021": true
    },
    "globals": {
        "L": true,
        "module": false,
        "define": false
      },
    "extends": [
        "eslint:recommended",
        "plugin:@typescript-eslint/recommended"
    ],
    "overrides": [
    ],
    "parser": "@typescript-eslint/parser",
    "parserOptions": {
        "ecmaVersion": "latest",
        "sourceType": "module"
    },
    "plugins": [
        "@typescript-eslint"
    ],
    "rules": {
    }
}
