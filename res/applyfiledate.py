#!/usr/bin/python3

import os
os.system("""echo "select photos.id, pathname, filename from photos inner join folders on photos.folder==folders.id where capturedate<'1981-01-01T00:00:00';" | sqlite3 ~/.local/share/photohoard/default.db > /tmp/photos.txt""")

with open("/tmp/photos.txt", "r") as fd:
    lines = fd.readlines()

sql = []
for line in lines:
    pid, path, fn = line.split("|")
    st = os.stat(f"{path}/{fn.strip()}")
    dt = time.localtime(st.st_mtime)
    dt = f"{dt.tm_year}-{dt.tm_mon:02d}-{dt.tm_mday:02d}T00:00:00"
    sql.append(f"update photos set capturedate='{dt}' where id=={pid};");

with open("/tmp/update.sql", "w") as fd:
    for k in sql:
        fd.write(f"{k}\n");
        
