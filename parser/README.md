# Data acquisition tool for de-law-explorer

A tool written in Ruby 2 to perform the following tasks:

1. Download source files.
2. Scan source files and put them to a SQLite database.
3. Scan the SQLite database and link texts accordingly.

**Warning**: Instructions below written dry and from fading memory. The
code has not been tested after 2015. Thus, it might not working on
current Ruby v2.x, dependencies might have abandoned or contain security
issues.

The code contains downloader and parsers working un untrusted sources,
so use is at your own risk!

This is code is for demonstrational purposes only.

## Setup

```
$ gem install bundler
$ bundle install
```

## Use

For setting up a law text database:

```
$ bundle exec ruby downloader.rb gesetze_im_internet ./xml
$ bundle exec ruby scan_all.rb ./xml/**/**.xml --scanner=juris_de --domain=BRD
$ bundle exec ruby install_keys.rb linker
$ bundle exec ruby linker.rb --linker=de --domain=BRD
$ bundle exec ruby install_keys.rb post_linker
$ bundle exec ruby static_links_gen.rb
```

The resulting `test.db` contains the data required to run
de-law-explorer.

Setup may take an hour or two (excluding downloads). For unknown reasons, setup
speed varied strongly by platform and time of setting up database keys.

## Data inspection

Printing out an HTML file to `output/` for a given book, useful for debugging
parser results.

```
$ bundle exec ruby restore.rb ustg
```

## License

GPL3, see ../LICENSE for full details.
