# Spec Delta

## Purpose

Defines how the plugin loads, preserves, and initializes its configuration (`MarkdownView.ini`) across installs and updates.

## ADDED Requirements

### Requirement: Sample file as fallback layer
The system SHALL resolve every configuration setting through three layers, in this order: the user's `MarkdownView.ini`, the shipped `MarkdownView.ini.sample` in the same directory, and the hardcoded default at the read site. The first layer that contains the section/key wins; a key that is missing from both files resolves to the hardcoded default.

#### Scenario: Key overridden by the user
- **WHEN** a key is present in the user's `MarkdownView.ini` and in the shipped sample
- **THEN** the value from the user's `MarkdownView.ini` is used

#### Scenario: Key inherited from the sample
- **WHEN** a key is missing from the user's `MarkdownView.ini` but present in the shipped sample
- **THEN** the value from the shipped sample is used

#### Scenario: Key absent from both files
- **WHEN** a key is missing from the user's `MarkdownView.ini` and from the shipped sample
- **THEN** the hardcoded default at the read site is used

#### Scenario: Sample file missing
- **WHEN** the shipped `MarkdownView.ini.sample` does not exist
- **THEN** settings are read from the user's `MarkdownView.ini` only, and keys missing from it resolve to the hardcoded default, without a warning

#### Scenario: Plugin directory is read-only
- **WHEN** `MarkdownView.ini` cannot be created because the plugin directory is read-only, while `MarkdownView.ini.sample` is present
- **THEN** all settings resolve through the shipped sample and no configuration file is written
