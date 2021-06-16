# ESP Matter

## Getting started

### Clone and bootstrap

```bash
$ cd ${HOME}
$ git clone https://github.com/espressif/esp-matter.git
$ cd ${HOME}/esp-matter
$ git submodule update --recursive --init
$ source ${HOME}/esp-matter/connectedhomeip/scripts/bootstrap.sh
$ ${HOME}/esp-matter/esp-idf/install.sh
$ sudo apt-get install ninja-build
```

### Environment setup

```bash
$ source ${HOME}/esp-matter/export.sh            
$ source ${HOME}/esp-matter/esp-idf/export.sh            
```
