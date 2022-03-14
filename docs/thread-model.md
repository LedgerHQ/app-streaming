## Security Objectives

1. Provide a mechanism to ensure that only legit apps are executed.
2. Guarantee the authenticity, integrity and confidentiality of code and data during an app execution.
3. Isolation of derivation paths.


## Encrypted App Overview

```
 app.zip
┌────────────────────────────────────────────────────────┐
│                                                        │
│ ┌───────────────────┐                                  │
│ │ manifest.bin      │                                  │
│ │                   │ [ encrypted using DevicePubKey   │
│ │- KeyAES1          │   signed using LedgerPrivKey  ]  │
│ │- KeyHMAC1         │                                  │
│ │- DerivationPaths  │                                  │
│ └───────────────────┘                                  │
│                                                        │
│ ┌───────────────────┐                                  │
│ │ code.bin          │                                  │
│ │                   │ [ encrypted and authenticated    │
│ │                   │   with KeyAES1 and KeyHMAC1   ]  │
│ │                   │                                  │
│ │                   │                                  │
│ └───────────────────┘                                  │
│                                                        │
│ ┌───────────────────┐                                  │
│ │ data.bin          │                                  │
│ │                   │ [ encrypted and authenticated    │
│ │                   │   with KeyAES1 and KeyHMAC1   ]  │
│ │                   │                                  │
│ │                   │                                  │
│ └───────────────────┘                                  │
│                                                        │
└────────────────────────────────────────────────────────┘
```


## Security Mechanisms

### Static encryption and authentication keys (code and data)

An app is a static 32-bit RISC-V ELF binary with 2 sections: code (read-only) and data (read-write). These sections are divided into pages of 256 bytes. A page has an address (4 bytes since ELF are 32-bit binaries) and a counter (4 bytes) initialized to 0.

Every pages are encrypted using a static set of 2 keys:

- `KeyAES1`: AES-256. The IV is `addr || counter || '\x00' * 8` where `addr` and `counter` are 4 bytes each and `counter` is 0.
- `KeyHMAC1`: authentication using HMAC-256 (`encrypted_data || addr || counter`).

A random set of keys is generated for each app and each device by a Ledger HSM. In the case where a set of keys is compromised, an attacker would be able do decrypt the code and data of the app (`KeyAES1`) and forge malicious code or data (`KeyHMAC1`). However, it'd be limited to the single app and device using this set of key.

This set of symmetric keys is embedded in the app manifest (unique to each app and each device).

### Dynamic encryption and authentication keys (modified data, stack and heap)

A dynamic set (`KeyAES2`, `KeyHMAC2`) is initialized randomly by the VM each time an app is launched and a merkle tree keeps track of the counter associated to a page to prevent replay.

This set of keys works in a similar way than the code and data encryption and authentication mechanism, but is applied to writeable data (stack, heap and modified data).

This set of keys never leaves the device.

### Signed and encrypted manifests

Security objective: legit apps.

The symmetric keys (`KeyAES1` and `KeyHMAC1`) which guarantee the security of the execution of an app are embedded in the manifest. These keys are generated randomly by Ledger HSMs, per app and per device. These symmetric keys aren't expected to become public. However, if a malicious VM app retrieved these keys, it would only impact the app and the device for which this manifest was generated.

This manifest is encrypted using a public key of the device (`DevicePubKey`) and signed using Ledger private key (`LedgerPrivKey`). It guarantees that only the device for which the manifest is encrypted can decrypt these symmetric keys. A unique `DevicePubKey` could be derive for each version of each app.

The signature can only be created by Ledger HSMs. It prevents an attacker from forging malicious manifest or from knowing the keys inside this manifest.

### Isolation of derivation paths

A new syscall should be introduced to BOLOS to restrict the derivation paths allowed by an app during it's execution.

The derivation paths should be specified in the app manifest.
