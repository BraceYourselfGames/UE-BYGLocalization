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

For this example, we will be using English as the **Original Language**, but
the system works with using any language as the original language.

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


### Getting Localized Text in C++ 

```cpp
FText ButtonLabelText = UBYGLocalizationStatics::GetGameText( "Hello_World" );
```

### Changing the active locale

FString PathToCSV;
UBYGLocalizationStatics::SetActiveLocalization( 


## Installation

### Source

1. Download the zip or clone the repository to `ProjectName/Plugins/BYGLocalization`.
2. Add `BYGLocalization` to `PrivateDependencyModuleNames` inside `ProjectName.Build.cs`.

## Unreal Version Support

* Unreal Engine 4.26
* May work with previous versions, but not tested

## License

* [3-clause BSD license](LICENSE)

## Contact

* Created and maintained by [@_benui](https://twitter.com/_benui) at [Brace Yourself Games](https://braceyourselfgames.com/)


