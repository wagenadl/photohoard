#!/usr/bin/python3

import sqlite3
import os
import shutil
import uuid

'''This loads the database x.db and creates a new database z.db
containing only one folder tree from x.db. It then removes that 
folder tree from x.db. A copy of the original x.db is kept as y.db.
'''

xfn = "/home/wagenaar/.local/share/photohoard/default.db"
yfn = "/home/wagenaar/.local/share/photohoard/default.db.bk"
zfn = "/home/wagenaar/.local/share/photohoard/split.db"

zpath = "/home/wagenaar/split-this-folder"

shutil.copyfile(xfn, yfn)
shutil.copyfile(xfn, zfn)

zcon = sqlite3.connect(zfn)
zcur = zcon.cursor()
zcur.execute("pragma foreign_keys = on")
zcur.execute("delete from folders where pathname!=? and parentfolder is null",
             [zpath])
zuid = uuid.uuid4()
zcur.execute("update info set val=? where id=='databaseid'", [str(zuid)])
zcon.commit()

xcon = sqlite3.connect(xfn)
xcur = xcon.cursor()
xcur.execute("pragma foreign_keys = on")
xcur.execute("delete from folders where pathname==?",
             [zpath])
xcon.commit()
