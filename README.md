# BYG Localization

We wanted to support fan localization for [Industries of
Titan](https://braceyourselfgames.com/industries-of-titan/) and found that
Unreal's built-in localization system was not exactly what we wanted. So we
made our own!

It differs from Unreal's localization system in a few ways:

* Only support a single CSV file as the authoritative source for strings.
* Support creation and maintenance of fan translations.
* Fallback to primary language if text is missing in a fan translation.
* Clearer errors when keys are missing in a stringtable.
* Support multiple localizations for the same language.



## Feature Comparison

| Feature | Unreal Localization | BYG Localization |
| --- | --- | --- |
| CSV stringtable support							| :heavy_check_mark: 	| :heavy_check_mark:	|
| Add new keys in-editor							| :heavy_check_mark:	| :x:					|
| Reload text on CSV modification					| :heavy_check_mark: 	| :heavy_check_mark:	|
| Support for fan translations						| :x:					| :heavy_check_mark:	|
| Show missing loc keys in-engine					| :x:					| :heavy_check_mark:	|
| Show fallback language text when keys are missing | :x:					| :heavy_check_mark:	|
| Blueprint code support							| :heavy_check_mark:	| :heavy_check_mark:	|
| Multiple localizations for same language			| :question:			| :heavy_check_mark:	|



## Usage

For this example, we will be using English as the **Primary Language**, but
the system works with using any language as the Primary language.

### Create CSV file

We are using English as the primary language for our game so we will create
`loc_en.csv` inside `/Content/Localization/`, the default localization root
directory for localization files. 

```
Key,SourceString,Comment,English,Status
Hello_World,"Hello world, how are you?",General greeting.,,
Goodbye_World,"See you later!",Shown when quitting the game.,,,
```

### Using Localized Text in Blueprints

After adding the keys to the Stringtable CSV file, choose the entries for all
`FText` properties by:

1. Click on the drop-down arrow.
2. Choose the Stringtable ID. BYG Localization defaults to "Game".
3. Choose the key

![Animation showing process for choosing a string entry](https://benui.ca/assets/unreal/stringtable.gif)

In Blueprint graphs, use `GetGameText` in `BYGLocalizationStatics`.

### Getting Localized Text in C++ 

```cpp
FText ButtonLabelText = UBYGLocalizationStatics::GetGameText( "Hello_World" );
```

### Changing the active locale

```cpp
FString PathToCSV;
UBYGLocalizationStatics::SetActiveLocalization( PathToCSV );
```

### Stats Window

There is an stats window available in the editor for seeing which localization
files have been detected by the system, how many entries they have, the status
of those entries etc.

Access it through `Window > Developer Tools > BYG Localization Stats`.

![Stats window example](https://benui.ca/assets/unreal/byglocalization-statswindow.png)

### Customizing Settings

All of the project settings can be modified through `Project Settings > Plugins > BYG Localization` in the editor, or through
`Config/DefaultBYGLocalization.ini`

Settings include:
* Localization file directories (default `/Localization/`)
* Allowed filetypes (default `.csv` and `.txt`)
* Filename prefix/suffix (default `loc_` prefix, no suffix)
* Forcing quotation marks around all CSV values.



## User Experience for Fan Localizers

### Creating a new localization

1. Create a file with the two-character [ISO 639-1 language
   code](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes) and optional
   region suffix. e.g. if the primary file is `loc_en.csv` and you want to
   translate the game into French, create a file called `loc_fr.csv`
2. Launch the game with the `-UpdateLoc` flag (create a desktop shortcut).
3. Your `loc_fr.csv` file is now populated with all of the primary language
   strings.

After creating `loc_fr.csv` and running the game, your CSV file will look like
this:

| Key | SourceString | Comment | Primary | Status |
| --- | --- | --- | --- | --- |
| `NewGameButtonLabel` | New Game | On main menu, starts new game | New Game | New Entry |
| `ExitGameButtonLabel` | Quit | Exits the program | Quit | New Entry |

After translation, your CSV file should look like this. You can remove the "New Entry" text from the Status column:

| Key | SourceString | Comment | Primary | Status |
| --- | --- | --- | --- | --- |
| `NewGameButtonLabel` | Nouvelle partie | On main menu, starts new game. | New Game | _(blank)_ |
| `ExitGameButtonLabel` | Quitter | Exits the program. | Quit | _(blank)_ |

### Maintaining a localization

As the game is updated, strings will be added, removed or modified.
* New strings will be shown with the status "New Entry", and will show up in the Primary Language until they are translated.
* Modified will be shown with the status "Modified" and what the primary language text was before.
* Removed strings will be shown with the status "Deprecated", or automatically removed (depending on the project settings).

| Key | SourceString | Comment | Primary | Status |
| --- | --- | --- | --- | --- |
| `NewGameButtonLabel` | Nouvelle partie | On main menu, starts new game. | New Game | _(blank)_ |
| `ExitGameButtonLabel` | Quitter | On main menu, starts new game. | Quit game | Modified: Was 'Quit' |
| `LoadGameButtonLabel` | Load Game | Shows the load game screen. | Load Game | New Entry |



## Installation

### Source

1. Download the zip or clone the repository to `ProjectName/Plugins/BYGLocalization`.
2. Add `BYGLocalization` to `PrivateDependencyModuleNames` inside `ProjectName.Build.cs`.



## Unreal Version Support

* Compiles under Unreal Engine 4.22 up to 5.0EA
* Tested mostly with 4.25 and 4.26



## License

* [3-clause BSD license](LICENSE)



## Contact

* Created and maintained by [@_benui](https://twitter.com/_benui) at [Brace Yourself Games](https://braceyourselfgames.com/)
* Please report bugs through [GitHub](https://github.com/BraceYourselfGames/UE4-BYGLocalization/issues)



## Future Work

* Detecting runaway misquoted strings.
* Allowing multiple stringtables, e.g. `loc_en_ui.csv`, `loc_en_dialog.csv`.
* More tests!
* Profiling and performance improvements.
* Improve dir picker, see `DirectoryPathStructCustomization`



## How it works

The plugin uses Project Settings to search for localization files in the
specified directories. It uses Unreal's String Table system to register both
the primary language (e.g. English) and the user's preferred language (e.g.
French).

Fallback works in two ways:

1) If text is set with `UBYGLocalizationStatics::GetText(const FString&
KeyName)` and the key is not found, the key is then looked up in the fallback
table.

2) When running the game, all localization files are parsed and any missing
keys are added to non-primary localization files. This way `FText` properties
inside Blueprints  will still find keys in the Stringtable for the user's
selected locale.



## FAQ

### Q) I changed Stringtable Namespace and/or Stringtable ID and now my text is gone!

**A)** Changing **Namespace** or **Stringtable ID** will break all of the
`FText` strings in your Blueprints. You should only set these values once at
the start of the project, and not change them.

