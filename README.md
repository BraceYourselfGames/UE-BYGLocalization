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

All of the project settings can be modified through `Project Settings > Plugins
> BYG Localization` in the editor, or through
`Config/DefaultBYGLocalization.ini`

Settings include:
* Localization file directories (default `/Localization/`)
* Allowed filetypes (default `.csv` and `.txt`)
* Filename prefix/suffix (default `loc_` prefix, no suffix)
* Forcing quotation marks around all CSV values.

## Installation

### Source

1. Download the zip or clone the repository to `ProjectName/Plugins/BYGLocalization`.
2. Add `BYGLocalization` to `PrivateDependencyModuleNames` inside `ProjectName.Build.cs`.

## Unreal Version Support

* Compiles under Unreal Engine 4.22~5.0EA
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

## FAQ

### Q) I changed Stringtable Namespace and/or Stringtable ID and now my text is gone!

**A)** Changing **Namespace** or **Stringtable ID** will break all of the
`FText` strings in your Blueprints. You should only set these values once at
the start of the project, and not change them.

