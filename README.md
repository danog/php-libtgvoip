# php-libtgvoip
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fdanog%2Fphp-libtgvoip.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fdanog%2Fphp-libtgvoip?ref=badge_shield)


Licensed under AGPLV3, created by Daniil Gentili.

This is a PHP extension that wraps the [telegram VoIP library](https://github.com/grishka/libtgvoip), created using php-cpp.

Note: this is a legacy project based on the old libtgvoip library and supports only PHP 7.4, it will be eventually rewritten to make use of the new tgcalls library and PHP 8.0+ with FFI.

## Installation

Install the libopus headers (libopus-dev), openssl headers and the php-cpp headers.
Then clone this repository recursively and run `make && sudo make install`

On Ubuntu/debian/any debian-based distro:


```
sudo apt-get install libopus-dev libssl-dev build-essential php7.4-dev php7.4-cli && git clone https://github.com/copernicamarketingsoftware/PHP-CPP && cd PHP-CPP && sed 's/php-config/php-config7.4/g' -i Makefile && make -j$(nproc) && sudo make install && cd .. && git clone --recursive https://github.com/danog/php-libtgvoip && cd php-libtgvoip && make && sudo make install
```

## Documentation

https://docs.madelineproto.xyz/docs/CALLS.html
