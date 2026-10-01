# Release Guide

## Local Candidate

1. Run `scripts/install.sh --skip-install`.
2. Complete the checklist in `COMPATIBILITY.md`.
3. Run `VERSION=1.0.0 scripts/package.sh`.
4. Inspect the archive in `dist/` and install it on a clean user account.

## Signing and Notarization

Import a Developer ID Application certificate, create a `notarytool` profile, then run:

```bash
CODESIGN_IDENTITY="Developer ID Application: Example (TEAMID)" \
NOTARY_PROFILE="ruindial-notary" \
VERSION=1.0.0 \
scripts/package.sh
```

The script signs each bundle, submits the archive, staples the accepted ticket, and recreates the distributable archive.

## GitHub Secrets

The tagged release workflow recognizes:

- `MACOS_CERTIFICATE`: base64-encoded `.p12` certificate
- `MACOS_CERTIFICATE_PASSWORD`: password for the certificate
- `CODESIGN_IDENTITY`: full Developer ID Application identity
- `APPLE_ID`: notarization Apple ID
- `APPLE_TEAM_ID`: Apple Developer team ID
- `APPLE_APP_PASSWORD`: app-specific password

Unsigned tagged builds still work when the secrets are absent, but they are not public release candidates.
