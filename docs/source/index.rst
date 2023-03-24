  .. image:: banner.svg
             :width: 700
             :align: left
             :class: no-scaled-link

Photohoard — Keeping your digital photos together
=================================================

If your photographic habits are at all like mine, you probably take
[#1]_ several thousand photos every year. You don't have the time (or
inclination) to organize them in a particular way, so they end up in
the digital equivalent of a shoebox. Then, your child's school needs a
few family pictures for some project and you end up desparately
searching through the folders. Very unpleasant, because even the
thumbnails are slow to load when you have thousands of them to look
at [#2]_.

Photohoard for organization
----------------------------


Photohoard is primarily intended to ease this pain. It does not force
you to spend any time organizing or tagging your photos (though you
can), but simply presents them in a timeline. It also caches previews
at various scales, so it is lightningly fast at scrolling through
thousands of images.

Photohoard allows you to apply “color labels”, “status flags”, “star
ratings,” and arbitrary “tags” to your images and even to organize
them into “collections.”  To be honest, I use the facilities
minimally. Just having all my photos in a timeline is the most
important thing.

Photohoard for nondestructive editing
-------------------------------------

In addition to organizing, Photohoard helps you develop your
photos. It contains a fully nondestructive image editing pipeline with
tools like:

- Cropping and rotating
- Perspective correction
- Exposure and curve correction
- White balance correction
- Unsharp masking and local contrast enhancement

All of the curve and color tools operate in appropriate color spaces
(mostly IPT [#3]_). Moreover, Photohoard is fully color-managed.

In practice, I usually apply edits to the entire image, although
Photohoard does offer regional masks so edits can be restricted to
particular areas.

Why use Photohoard?
-------------------

.. toctree::
   :maxdepth: 1
   :caption: Wondering if Photohoard is for you? Here is a comparison
             with some alternatives.

   why

User guide
------------

.. toctree::
   :maxdepth: 1
   :caption: For more details on how to use the software, read these chapters.

   install
   library
   browsing
   organization
   editing
   exporting
   technical
             
License information
-------------------

.. toctree::
   :maxdepth: 1
   :caption: Photohoard is free software. Read what that means here:

   license

Footnotes
----------

.. [#1] And, if you are lucky *make* a few as well.

.. [#2] This paragraph was written in 2018. You may now have your
        thousands of pictures on your phone, but the situation isn't
        much better, as that tiny screen is really not great for
        looking through a big pile of them.
        
.. [#3] Ebner, Fritz, 1998. *Derivation and modelling hue uniformity
        and development of the IPT color space.* Thesis. Rochester
        Institute of Technology. Accessed from ResearchGate.net.
