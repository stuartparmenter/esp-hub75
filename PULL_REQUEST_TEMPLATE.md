This pull request addresses inconsistencies in the Dependabot configuration.

Changes made:
1. Both `github-actions` and `uv` now use consistent commit message formats.
2. Both update types use the `dependencies` label uniformly.
3. The `major` group for the `uv` package-ecosystem has been removed to prevent major dependency upgrades from being treated differently through Release Drafter.