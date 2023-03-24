Technical information
=====================

The first time you start Photohoard, it creates a database in
$HOME/.local/photohoard/default.db. At present, it is not possible to change
that location, although you can move the file somewhere else and use a
link to help Photohoard find it. The database file doesn't get overly
big: 14 MB for my collection of 55,000 photos.

Photohoard also creates a cache for previews in
$HOME/.cache/photohoard/default-cache. This does get fairly big:
6 GB for my collection. Nevertheless, it is helpful to have the cache
on a fast medium like an SSD.

Photohoard can, in principle, reconstruct the cache if it gets lost,
though it can take many hours. The database file is irreplaceable
and should be included in your regular backup regimen.

