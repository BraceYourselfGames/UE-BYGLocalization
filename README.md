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


## Feature Comparison

| Feature | Unreal Localization | BYG Localization |
| --- | --- | --- |
| CSV Stringtable		| :heavy_check_mark: 		| :heavy_check_mark:	|
| In-editor keys					| :heavy_check_mark:	| :x:	|
| Dynamic reload in editor		| :heavy_check_mark: 		| :heavy_check_mark:	|
| Support for fan translations		| :x: 		| :heavy_check_mark:	|
| Show missing loc keys in-engine | :x: 		| :heavy_check_mark:	|
| Show fallback language text when keys are missing | :x: 		| :heavy_check_mark:	|
| Blueprint code support		| :heavy_check_mark:	| :heavy_check_mark:					|


## Usage

For this example, we will be using English as the **Original Language**, but
the system works with using any language as the original language.

## Installation

### Source

1. Download the zip or clone the repository to
   `ProjectName/Plugins/BYGLocalization`.
2. Add `BYGLocalization` to `PrivateDependencyModuleNames` inside `ProjectName.Build.cs`.

## Unreal Version Support

* Unreal Engine 4.26
* May work with previous versions, but not tested

## License

* [3-clause BSD license](LICENSE)

## Contact

* Created and maintained by [@_benui](https://twitter.com/_benui) at [Brace Yourself Games](https://braceyourselfgames.com/)


