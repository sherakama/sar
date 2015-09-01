How do you use this?

Place the sar directory in ~/.drush/commands (create it if you don't have one)
and run `drush help sar'.

Use `drush sar "replace me" "new text"' to replace all occurrences of
"replace me" with "new text" in all text fields (node body and field API) and
all custom blocks on the site.

Be sure to backup your database before running this command, as that is the
only way to undo changes.

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
