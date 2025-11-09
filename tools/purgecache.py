#!/usr/bin/python3

import sqlite3
import os
import shutil
import uuid

'''This loads the database x.db and drops all cache entries that do
not correspond to actually existing versions.'''

xfn = "/home/wagenaar/.local/share/photohoard/default.db"
xcon = sqlite3.connect(xfn)
xcur = xcon.cursor()
xuid = xcur.execute("select val from info where id=='databaseid'").fetchone()[0]

cachedir = f"/home/wagenaar/.cache/photohoard/{xuid}-cache"
xcur.execute("attach ? as cache", [f"{cachedir}/cache.db"])
dbnos = [x[0] for x in
         xcur.execute("select distinct dbno from cache.cache where dbno>0")
         .fetchall()]
for dbno in dbnos:
    xcur.execute("attach ? as ?", [f"{cachedir}/blobs{dbno}.db",
                                   f"blobs{dbno}"])
xcur.execute("attach ':memory:' as M")

for dbno in dbnos:
    xcur.execute(f"delete from blobs{dbno}.blobs where cacheid in (select id from cache.cache where dbno==? and version not in (select id from versions))", [dbno])
    
rows = xcur.execute("select version, maxdim from cache.cache where dbno==0 and version in (select id from versions)").fetchall()

def purgejpeg(dirpath, base, rows):
    fns = os.listdir(dirpath)
    for fn in fns:
        if os.path.isdir(f"{dirpath}/{fn}"):
            purgejpeg(f"{dirpath}/{fn}", 100*base + int(fn), rows)
        elif fn.endswith(".jpg"):
            fn1 = fn.split(".")[0]
            nn, xx = fn1.split("-")
            version = 100*base + int(nn)
            maxdim = int(xx)
            if (version, maxdim) not in rows:
                print("Removing", version, maxdim, dirpath, fn)
                os.remove(f"{dirpath}/{fn}")

purgejpeg(f"{cachedir}/thumbs", 0, rows)

xcur.execute("delete from cache.cache where version not in (select id from versions)")
xcur.execute("delete from cache.queue where version not in (select id from versions)")
xcon.commit()

