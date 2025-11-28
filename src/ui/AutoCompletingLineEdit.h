// AutoCompletingLineEdit.h

#ifndef AUTOCOMPLETINGLINEEDIT_H

#define AUTOCOMPLETINGLINEEDIT_H

#include <QLabel>
#include <QVariant>
#include <QStringList>

class AutoCompletingLineEdit: public QLabel {
public:
  AutoCompletingLineEdit(QWidget *parent);
  virtual ~AutoCompletingLineEdit();
public:
  virtual QStringList optionsStartingWith(QString) const;
  virtual QStringList optionsContaining(QString) const;
  virtual bool exists(QString) const;
public:
  void setUniverse(QStringList);
  /* If deriving class overrides optionsXXX() and exists(), the
     universe is not used. */
  void allowMultiple(bool);
  /* Allow separate completion in bits separated by space or special
     characters */
  void allowNonexistent(bool);
  /* Allow accepting string not in the options */
  bool acceptable() const;
  bool acceptable(QString) const;
  bool acceptable(QVariant const &) const;
protected:
  void resuggest();
  /* Rebuild suggestion box */
  void unsuggest();
  /* Close suggestion box */
  void recomplete();
  /* Use suggestion box to rebuild richtext */
  void acceptcompletion();
  /* Use suggestion box to update usertext */
  QVariant parse() const;
  /* If allowing multiple, this parses into a lisp-style structure.
     For instance
       "A | (B & C | D) & E"
     would parse into
       ("&", ("|", "A", ("|", ("&", "B", "C"), "D")), "E").
     That is:
       PARSED = STR
                (OP PARSE PARSE)
     Accepted operators are "&" and "|".
     Spaces around operators are dropped.
     Priority is strictly left to right, except for parens.
     Returns null variant in case of syntax error.
  */
signals:
  void enterPressed(QString);
  /* Only emitted if the string passes muster per "allow" config */
  void escapePressed();
  /* Only emitted if the Escape press was not consumed by dropping
     suggestions. */
protected:
  void paintEvent(QPaintEvent *) override;
  void keyPressEvent(QKeyEvent *) override;
  /* We support:
     
     Left: Cursor left
     Right: Cursor right or expand suggestion if at edge
     Shift + Left / Right: Cursor left/right selecting
     Ctrl + Left / Right: Cursor to begin/end of text
     Up / Down: Navigate suggestions
     Tab: Expand suggestion
     Enter: Expand suggestion and accept
     Escape: Drop suggestions or cancel edit if none
     Ctrl + A: Select all
     Ctrl + X / C / V: Cut/copy/paste
     Backspace: Backward delete or delete selection
     Delete: Forward delete or delete selection
     Ctrl + K: Delete to end-of-line and drop suggestions
     Ctrl + Z: Undo

     Other key combos are accepted to the extent they produce text.
     Note that some Ctrl + letter combos surprisingly produce text
     even on my US English keyboard. I guess that's OK.

     Undo is useful after inadvertent acceptance of suggestion. I
     don't see an immediate need for Redo.
  */
  /* There are two cases to consider for constructing autocomplete:
     (1) multiple + space sep;
     (2) everything else.

     (2) is easy: we just break up the text at obvious separation and
     see if the current "bit" yields suggestions. Options "starting
     with" take priority. If any exist, options "containing" are not
     shown.

     (1) is more complicated. The most sensible thing might be: split
     the bit into space-separated words. From most inclusive to least
     inclusive, try whether options "starting with" exist. Then repeat
     with options "containing". This scales with O(N K^2) in the
     number of words K and number of options in the universe N. This
     could become problematic if the optionsXXX() methods are
     slow. But if those methods simply check the ``universe`` string
     list, I expect it to be fine for the case where N is the number
     of tags in the photodb and K at least 5. We could set a time
     limit for the search.

     We could pre-peel any words before and after that exist() as
     separate items in the universe. Whether that is a good idea
     depends on how we plan to interpret the final string. One
     reasonable interpretation: greedily pick the longest prefix list
     of words that exists in the universe, in which case pre-peeling
     is not reasonable. Alternatively, if we greedily pick the
     shortest prefix list, pre-peeling makes sense. A separate issue
     is whether we should backtrack if an initially plausible
     interpretation hits a dead end. That seems excessive. Let the
     user use commas to disambiguate.

     Note that we _need_ to be able to perform interpretation in order
     to implement acceptable(). Perhaps we should return a syntax tree?
     That is, a method to convert, e.g.,

        Tag1 Tag2 | Tag3

     into

        ("|", ("", Tag1, Tag2), Tag3).
        
     (This clearly is most easily represented in Lisp. In C++, it's a
     mess. Perhaps just a QVariant either expanding to a 3-element
     QVariantList or QStrings, where the 3-element list comprises a
     QString followed by the same thing allowed on top. Is it easy to
     define a "LimitedVariant" and "LimitedVariantList" to express
     this?)

     One thing is clear, the space separator should be the most
     tightly binding operator.

     Note that in _search_ context, booleans make sense. In _apply_
     context, they don't. Likewise, in _search_ space sep might make
     sense, in _apply_ context allow-multiple is not needed. Finally,
     in _search_, there is no need to allow-nonexistent, but there
     might be use for wildcards.

     Actually, I think I may settle on this:
     - In _apply_ context, do not allow multiple;
     - In _search_ context, allow multiple but no space sep;
     - For the list, use commas.
   */
private:
  struct State {
    QString usertext;
    int cursoridx;
    int anchoridx;
  public:
    State() {
      cursoridx = 0;
      anchoridx = -1'
    }
  };
private:
  bool allowmulti;
  bool allownonexist;
  QStringList universe;
  State state;
  QList<State> undolist;
  QString richtext;
  bool cursorblink;
  class ListWidget *suggestionbox;
};

#endif
