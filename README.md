# php-libtgvoip

Licensed under AGPLV3, created by Daniil Gentili.

This is a PHP extension that wraps the [telegram VoIP library](https://github.com/grishka/libtgvoip), created using php-cpp.

Installation:

Install the libopus headers (libopus-dev), binutils (binutils-dev), openssl headers and the php-cpp headers.
Then clone this repository recursively and run `make && sudo make install`

On Ubuntu/debian/any debian-based distro:


```
sudo apt-get install libopus-dev binutils-dev libssl-dev
git clone https://github.com/CopernicaMarketingSoftware/PHP-CPP
cd PHP-CPP
make -j$(nproc)
sudo make install
cd ..
git clone --recursive https://github.com/danog/php-libtgvoip
cd php-libtgvoip
make -j$(nproc)
sudo make install
```




