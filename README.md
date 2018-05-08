# How to Use This Plugin

Place the sar directory in `~/.drush/commands` (create it if you don't have one)
and run `drush help sar`.

Use `drush sar "replace me" "new text"` to replace all occurrences of
"replace me" with "new text" in all text fields (node body and field API) and
all custom blocks on the site.

**Be sure to backup your database before using this plugin, as that is the
only way to undo changes.**

## Link Fields

```
Examples:
 simple     drush sarl devel.example.com www.example.com
 fields     drush sarl --fields=field_name,field_other devel.example.com www.example.com

Arguments:
 search     Existing text.
 replace    Replacement text.

Options:
 --fields   Only perform the search and replace on these named fields.
```

## JS Injector rules

```
Examples:
 simple                                    drush sarjs devel.example.com www.example.com                   
 regex                                     drush sarjs devel.example.com www.example.com --regex=^https://

Arguments:
 search                                    Existing text.    
 replace                                   Replacement text.

Options:
 --regex                                   Use regular expression when searching for a match
 
```
 
## Menu Items

```
Examples:
 simple     drush sarm devel.example.com www.example.com
 fields     drush sarm --menus=main-menu,navigation devel.example.com www.example.com

Arguments:
 search     Existing text.
 replace    Replacement text.

Options:
 --menus   Only perform the search and replace on these named menus. Leave empty for all.
```

## Views Headers and Footers

```
Examples:
 simple                                    drush sarv devel.example.com www.example.com                  
 regex                                     drush sarv --regex "([a-z0-9]).example.com" "\$1.example.org"

Arguments:
 search                                    Existing text.    
 replace                                   Replacement text.

Options:
 --regex                                   Use regular expression when searching for a match
 
```

## A Few Notes About Regular Expressions

1. If you encounter a tricky search-and-replace and decide to pass the `--regex` option, now you have two problems.
2. `--regex` on Link fields, JS Injector rules, and Menu items (`sarl`, `sarjs`, and `sarm`)uses the [REGEXP functionality in MySQL](https://dev.mysql.com/doc/refman/8.0/en/regexp.html). That means that it operates only on database fields that match what is passed to `--regex`. I.e., your search string and your `--regex` string can be completely different. You **must** pass an argument to `--regex=`.
3. `--regex` on Views (`sarv`) allows you to use regular expressions *within your search string*, as it uses [preg_replace()](https://secure.php.net/manual/en/function.preg-replace.php).