
Establishing your library
=========================

Photohoard keeps metadata about your pictures (including timestamps,
exposure information, and camera details) as well as all your edits in
a database file. It does not touch your original image files
[#1]_. This database needs to be setup the first time you use the
program. We'll assume that you already have a set of shoeboxes full of
photos. Perhaps they are stored in various subfolders of
/home/me/Pictures as well as in /data/more/Photos. The first time you
run Photohoard, you need to tell it about these locations by clicking
the “Add new folder tree” icon in the top left of the window (or by
pressing Control+Shift+R). You can do this for as many locations as
you want. Subfolders are automatically included. And you only have to
do it once.

Adding a large tree to Photohoard takes a while, because Photohoard
reads every single image file to extract metadata and to create small
preview images. This operation completes in the background, so you
don't need to wait for it.

Note that Photohoard does not copy or move your original image
files. It simply creates references to them in its database. Just to
be clear: That means that you must not delete the original files. (If
you do delete them, Photohoard will drop them from its database next
time you rescan the tree; more on this below.)

Adding more photos
-------------------

There are several ways to add more photos to Photohoard.

From a camera
^^^^^^^^^^^^^^

If your photos are on a card from a digital camera, the easiest thing
to do is to insert that card in your computer, make sure it gets
mounted (which GNOME and Cinnamon do automatically), and click the
“Import from camera or card” icon (or press Control+I). A dialog
window pops up in which you can choose where the photos will be
stored. (If Photohoard complains that “No removable media can be
found,” it usually suffices to open the card in a Files window.)

This same procedure also works for many cameras if you attach it to
your computer with a USB cable.

In the “Import” dialog, you can also specify that Photohoard should
delete the files from the card after importing, or move them to a
backup location. This location is a folder called
“photohoard-backup” in the root of your card. (This is my preferred
method. I delete that backup just prior to the next time I import
photos, in the secure knowledge that I have backed up my laptop since
the time I previously imported photos.)

Photohoard cannot handle movie files, but if there are any movie files
on your card, it does offer to move them over to your computer. (I have
to admit I still keep movie files in a digital cardboard box.)

From elsewhere
^^^^^^^^^^^^^^^

If your image files are already on your computer, you can simply drag
them into Photohoard. You will be given the choice of adding their
location to the set of “folder trees,” or to move the files into one of
the previously incorporated trees.

Alternatively, you can place the images inside a folder tree that
Photohoard already knows about. Photohoard will not automatically
discover that you did that, but you can tell it to check all of its folder
trees for new or modified files by clicking the “Rescan folders”
icon (or pressing Control+R). This operation will take a while to
complete, but runs in the background so you don't have to wait for it.

Of course, you can also add a new folder tree to Photohoard at any
time.

Footnote
--------------------

.. [#1] Except during the :ref:`“Purge Rejects” <purge>` operation
