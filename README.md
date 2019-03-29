# php-libtgvoip
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fdanog%2Fphp-libtgvoip.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fdanog%2Fphp-libtgvoip?ref=badge_shield)


Licensed under AGPLV3, created by Daniil Gentili.

This is a PHP extension that wraps the [telegram VoIP library](https://github.com/grishka/libtgvoip), created using php-cpp.

## Installation

Install the libopus headers (libopus-dev), openssl headers and the php-cpp headers.
Then clone this repository recursively and run `make && sudo make install`

On Ubuntu/debian/any debian-based distro:


```
sudo apt-get install libopus-dev libssl-dev build-essential php$(echo "<?php echo PHP_MAJOR_VERSION.'.'.PHP_MINOR_VERSION;" | php)-dev && git clone https://github.com/CopernicaMarketingSoftware/PHP-CPP && cd PHP-CPP && make -j$(nproc) && sudo make install && cd .. && git clone --recursive https://github.com/danog/php-libtgvoip && cd php-libtgvoip && make && sudo make install
```

## Documentation

https://docs.madelineproto.xyz/docs/CALLS.html
