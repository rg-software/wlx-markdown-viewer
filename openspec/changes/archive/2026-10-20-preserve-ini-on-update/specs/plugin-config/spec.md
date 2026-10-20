# Spec Delta

## Purpose

Defines how the plugin loads, preserves, and initializes its configuration (`MarkdownView.ini`) across installs and updates.

## ADDED Requirements

### Requirement: Preserve existing configuration on update
The system SHALL NOT overwrite an existing `MarkdownView.ini` when a newer version of the plugin is installed.

#### Scenario: Update with existing INI
- **WHEN** the plugin is installed or updated into a directory that already contains `MarkdownView.ini`
- **THEN** the existing `MarkdownView.ini` remains unchanged

### Requirement: Create default configuration on first run
The system SHALL create `MarkdownView.ini` from the shipped template when it is missing at plugin startup.

#### Scenario: First run without INI
- **WHEN** the plugin starts and `MarkdownView.ini` does not exist
- **THEN** a default `MarkdownView.ini` is created from the shipped template before settings are read

### Requirement: Ship configuration as template
The release package SHALL include the default configuration as a template file rather than as `MarkdownView.ini`.

#### Scenario: Package contents
- **WHEN** a release archive is produced
- **THEN** it contains the configuration template (e.g. `MarkdownView.ini.sample`) and does not contain `MarkdownView.ini`