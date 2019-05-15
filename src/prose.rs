//! Defines a piece of Prose and its associated data-structures and functions.
//! This is based heavily on Pandoc's AST (found in the pandoc-types hackage
//! package); however, one key difference is that the body of the AST doesn't
//! actually contain any text; it only contains byte offsets into the text
//! of a piece of Prose. This is to allow linters maximum flexibility if they
//! need access to the full text while keeping performance high.

/// A piece of prose, containing the full text and an AST which
/// references byte positions in the original text.
#[derive(Clone)]
pub struct Prose<'s> {
    pub text: &'s str,
    pub ast: Vec<Block<'s>>,
}

impl<'s> Prose<'s> {
    pub fn plain(text: &'s str) -> Prose<'s> {
        Prose {
            text,
            ast: Vec::new(),
        }
    }
}

/// A grouping of inline elements.
#[derive(Clone)]
pub enum Block<'s> {
    Para(Vec<Inline<'s>>),
    BlockQuote(Vec<Block<'s>>),
    Header(u8, Vec<Inline<'s>>),
}

#[derive(Clone)]
pub enum Inline<'s> {
    Str(Region<'s>),
    Emph(Region<'s>, Vec<Inline<'s>>),
    Strong(Region<'s>, Vec<Inline<'s>>),
    Strikeout(Region<'s>, Vec<Inline<'s>>),
    Superscript(Region<'s>, Vec<Inline<'s>>),
    Subscript(Region<'s>, Vec<Inline<'s>>),
    SmallCaps(Region<'s>, Vec<Inline<'s>>),
}

#[derive(Copy, Clone)]
pub struct Region<'s> {
    pub text: &'s str,
    pub start: usize,
    pub end: usize,
}
