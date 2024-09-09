***"Plugin IDs 1-1000 are reserved for development use and plugins must not be released publicly***
***with these IDs as clashes are likely. Plugin ID 0 will never be allocated."***

***"It is hoped that plugin IDs will fit into 24bits. In the unlikely event that this range needs***
***to be extended it will be [...] In the meantime hosts may assume that IDs will not exceed
0x00FFFFFF.***

https://www.ladspa.org/ladspa_sdk/unique_ids.html

---

Lots of these plugins actually skip a slot or two, but since the upper limit is 16777215
(16 million), I don't think we're in a rush to fill those, so I'll name approximate ranges,
skipping two or three empty slots.

| ID range | Plugin suite |
| - | - |
| 1051-1099, 1123, 1221-1227, 1841-1849 | CMT Library |
| 1181-1220, 1337, 1401-1440, 1605, 1881-1916 | SWH |
