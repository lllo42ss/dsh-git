# Security policy

## Scope

This is a student competition prototype. It controls motors and a servo, so a
software fault can cause physical motion. Treat every change as safety-relevant.

## Reporting

Do not open a public issue for a vulnerability that exposes credentials or allows
remote control. Contact the maintainers privately and include:

- affected commit/tag;
- reproduction steps;
- whether the car is connected to a network;
- impact and suggested mitigation.

## Secrets

- Never commit API keys, Wi-Fi passwords, bot tokens, or private keys.
- Use `.env` locally; `.env` is ignored by Git.
- If a secret is committed, rotate it first, then rewrite history.

## Network exposure

The Wi-Fi AP and MJPEG stream are for the competition bench only. Do not expose
the car's network to the public internet.
