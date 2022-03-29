An app is a static 32-bit RISC-V ELF binary with 2 sections: code (read-only) and data (read-write).

These sections are divided into pages of 256 bytes. A page has an address (4 bytes since ELF are 32-bit binaries) and a counter (4 bytes) initialized to 0.

The encryption of an app by Ledger HSMs results into:

- A manifest which contain in particular the entrypoint address of the ELF binary, the app name and version, etc.
- The set of encrypted pages with their addresses.

The host (ie. the PC or the smartphone) isn't trusted and has no knowledge over the keys.


## Streaming overview

```
 Host                                  Device
──────────────────────────────────────────────

       Send the manifest
      ──────────────────────────────►

       Ask for the page containing
       the entrypoint given its
       address
      ◄──────────────────────────────

       Reply with the asked page
      ──────────────────────────────►

       Continue execution and ask for
       any page that isn't in the
       cache when required
      ◄──────────────────────────────

       Reply the asked page
      ──────────────────────────────►

       If the cache is full, commit
       a modified page to the host
      ◄──────────────────────────────

       Reply with the merkle tree
       proof
      ──────────────────────────────►

                  ...

       Notify the host that the app
       has exited
      ◄──────────────────────────────

──────────────────────────────────────────────
```


## Pages encryption

All pages exchanged between the host and the VM are:

- encrypted using AES-256-CBC. The page data (256 bytes) is encrypted. The IV is `addr || counter || '\x00' * 8` where `addr` and `counter` are 4 bytes each, encoded in little-endian.
- authenticated using HMAC-SHA256. The following message is authenticated: `encrypted_data || addr || counter` where `encrypted_data` is the page data (256 bytes) encrypted using AES followed by `addr` and `counter` which are 4 bytes each and encoded in little-endian.

There are 2 sets of 2 keys for AES encryption/decryption and HMAC authentication:

1. A static set (`KeyAES1`, `KeyHMAC1`) used to encrypt and authenticate the read-only pages of the app.
2. A dynamic set (`KeyAES2`, `KeyHMAC2`) initialized randomly by the VM each time an app is launched.


## Exchanges

With the exception of the manifest initially sent by the host, there are 2 kind of exchanges, always initiated by the VM:

- Requesting a page, given its address. The page data, counter, HMAC and merkle tree proof are returned by the host. Note that the merkle tree proof isn't replied for read-only pages (because the IV of read-only pages never changes and is always 0).
- Committing a writeable page. The page data, counter and HMAC are sent by the VM. Committing a page always increments its associated counter by 1. The host replies with the merkle tree proof.


## Manifest

The manifest contains:

- the entrypoint address
- the code, stack and data sections addresses
- the application name and version
- the merkle tree root hash, size and last entry


## VM page cache

The VM has a 3 sets of cache for code, stack and data pages.


## Different kind of pages

### Code and read-only data

Code and read-only data pages are encrypted and authenticated using the static key set. The counter of these pages is always `0` and can never change.

### Initialized data

Initialized data pages are encrypted and authenticated using the static key set. The counter of these pages is initialized to `0` and is incremented over time when the VM modifies these pages. The VM will then switch to the dynamic key set.

### Stack and heap

Stack and heap pages don't exist initially. They're created by the VM and encrypted and authenticated using the dynamic key set. The counter is initialized to `1`.


## Anti-replay

Thanks to encryption, an attacker has no knowledge over the content of pages. Thanks to authentication, an attacker can't forge invalid pages. However an attacker could replay valid pages that were later modified by the VM.

Each writeable page is part of a merkle tree whose root hash is kept by the VM. The merkle tree is initialized with the writeable pages from the app. Each time the VM commit a page, this page is either inserted in the merkle tree (for a new stack page or heap page) or the corresponding node in the merkle tree is updated.

The nodes of these merkle tree are made of 8 bytes of data associated to a page: `addr || counter`. It's guaranteed that there's a unique node in the tree associated to an address. (To prevent second preimage attacks -- as designed by Certificate Transparency, a 0x00 byte is prepended to the hash data of leaf node hashes, while 0x01 is prepended when computing internal node hashes.)

A merkle tree path for an address contains the shortest list of additional nodes in the tree required to compute the root hash for that tree. A node is either prefixed by the character `L` if it's the left child of its parent or `R` otherwise.

When the VM receives a writeable page from the host, a merkle tree path is used to ensure that the node made of the associated address and counter is actually part of the tree. It guarantees that the counter associated to the page is valid.
