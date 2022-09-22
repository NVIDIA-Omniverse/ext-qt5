## Public Qt mirror repository

This repo serves as a mirror for the complete Qt source, branched by version number. The copies are taken directly from https://github.com/qt/qt5 (with its corresponding version tags) and remain unmodified, but its submodules are flattened and included within the same version branch.

Licensing: the various version branches mirror all the original Qt license files (per version branch), but not all of those are always relevant for clients of Qt. The qt-LICENSE.txt file details which components of Qt are subject to which license. The following projects are using (part of) the Qt libraries/headers as (indirect) dependency, with the license(s) covering their Qt usecase:

| Client                              | Location                                   | Swipat #  | License          |
| ----------------------------------- | ------------------------------------------ | --------- | ---------------- |
| NVIDIA Omniverse ParaView Connector | Binary install as part of NVIDIA Omniverse | 200724292 | LICENSE.LGPL(v)3 |
