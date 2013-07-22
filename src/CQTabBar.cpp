#include <CQTabBar.h>

#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QStylePainter>
#include <QStyleOptionTab>
#include <QMouseEvent>
#include <QToolTip>

#include <cassert>

// create tab bar
CQTabBar::
CQTabBar(QWidget *parent) :
 QWidget(parent), currentIndex_(-1), position_(North), allowNoTab_(false),
 buttonStyle_(Qt::ToolButtonIconOnly), iconSize_(16,16), iw_(0), w_(0), h_(0),
 clipNum_(-1), offset_(0), pressed_(false), pressIndex_(-1), moveIndex_(-1)
{
  setObjectName("tabBar");

  setAcceptDrops(true);

  // add scroll buttons if tab bar is clipped
  lscroll_ = new CQTabBarScrollButton(this, "lscroll");
  rscroll_ = new CQTabBarScrollButton(this, "rscroll");

  connect(lscroll_, SIGNAL(clicked()), this, SLOT(lscrollSlot()));
  connect(rscroll_, SIGNAL(clicked()), this, SLOT(rscrollSlot()));

  lscroll_->hide();
  rscroll_->hide();
}

// delete tab bar
CQTabBar::
~CQTabBar()
{
  for (TabButtons::iterator p = buttons_.begin(); p != buttons_.end(); ++p)
    delete *p;
}

// add tab for widget with specified text
int
CQTabBar::
addTab(const QString &text, QWidget *w)
{
  return addTab(QIcon(), text, w);
}

// add tab for widget with specified text
int
CQTabBar::
addTab(const QIcon &icon, const QString &text, QWidget *w)
{
  // create button
  CQTabBarButton *button = new CQTabBarButton(this);

  button->setText  (text);
  button->setIcon  (icon);
  button->setWidget(w);

  return addTab(button);
}

// add tab for button
int
CQTabBar::
addTab(CQTabBarButton *button)
{
  int ind = count();

  insertTab(ind, button);

  return ind;
}

// insert tab for widget with specified text
void
CQTabBar::
insertTab(int ind, const QString &text, QWidget *w)
{
  insertTab(ind, QIcon(), text, w);
}

// insert tab for widget with specified text
void
CQTabBar::
insertTab(int ind, const QIcon &icon, const QString &text, QWidget *w)
{
  // create button
  CQTabBarButton *button = new CQTabBarButton(this);

  button->setText  (text);
  button->setIcon  (icon);
  button->setWidget(w);

  insertTab(ind, button);
}

void
CQTabBar::
insertTab(int ind, CQTabBarButton *button)
{
  button->setIndex(ind);

  buttons_.push_back(button);

  // update current
  if (! allowNoTab() && currentIndex() < 0)
    setCurrentIndex(ind);

  // update display
  updateSizes();

  update();
}

// remove tab for widget
void
CQTabBar::
removeTab(QWidget *widget)
{
  // get tab index
  int ind = getTabIndex(widget);
  assert(ind >= 0);

  removeTab(ind);
}

// remove tab at index
void
CQTabBar::
removeTab(int ind)
{
  // remove and delete button
  CQTabBarButton *button = buttons_.at(ind);

  buttons_[ind] = 0;

  delete button;

  // reset current if deleted is current
  if (currentIndex() == ind)
    setCurrentIndex(-1);

  // update display
  updateSizes();

  update();
}

// get number of tabs
int
CQTabBar::
count() const
{
  int n = 0;

  for (TabButtons::const_iterator p = buttons_.begin(); p != buttons_.end(); ++p) {
    CQTabBarButton *button = *p;

    if (button)
      ++n;
  }

  return n;
}

// set current tab
void
CQTabBar::
setCurrentIndex(int index)
{
  if (index < -1 || index >= count()) return;

  if (index != currentIndex_) {
    currentIndex_ = index;

    if (! allowNoTab() && currentIndex_ < 0 && count() > 0)
      currentIndex_ = 0;

    update();

    emit currentChanged(currentIndex_);
  }
}

// get tab index for specified widget
int
CQTabBar::
getTabIndex(QWidget *w) const
{
  for (TabButtons::const_iterator p = buttons_.begin(); p != buttons_.end(); ++p) {
    CQTabBarButton *button = *p;

    if (button && button->widget() == w)
      return button->index();
  }

  return -1;
}

// set tab location (relative to contents)
void
CQTabBar::
setPosition(const Position &position)
{
  // assert position is valid (just in case)
  assert(position == North || position == South || position == West || position == East);

  position_ = position;

  update();
}

// set allow no current tab
void
CQTabBar::
setAllowNoTab(bool allow)
{
  allowNoTab_ = allow;

  if (! allowNoTab() && currentIndex() < 0 && count() > 0)
    setCurrentIndex(0);
}

// set tab button style
void
CQTabBar::
setButtonStyle(const Qt::ToolButtonStyle &buttonStyle)
{
  buttonStyle_ = buttonStyle;

  updateSizes();

  update();
}

// set tab text
void
CQTabBar::
setTabText(int index, const QString &text)
{
  CQTabBarButton *button = tabButton(index);

  button->setText(text);

  updateSizes();

  update();
}

// set tab icon
void
CQTabBar::
setTabIcon(int index, const QIcon &icon)
{
  CQTabBarButton *button = tabButton(index);

  button->setIcon(icon);

  updateSizes();

  update();
}

// set tab tooltip
void
CQTabBar::
setTabToolTip(int index, const QString &tip)
{
  CQTabBarButton *button = tabButton(index);

  button->setToolTip(tip);
}

// set tab visible
void
CQTabBar::
setTabVisible(int index, bool visible)
{
  CQTabBarButton *button = tabButton(index);

  button->setVisible(visible);

  updateSizes();

  update();
}

// set tab pending state
void
CQTabBar::
setTabPending(int index, bool pending)
{
  CQTabBarButton *button = tabButton(index);

  button->setPending(pending);

  update();
}

// set tab data
void
CQTabBar::
setTabData(int index, const QVariant &data)
{
  CQTabBarButton *button = tabButton(index);

  button->setData(data);
}

// get tab data
QVariant
CQTabBar::
tabData(int index) const
{
  CQTabBarButton *button = tabButton(index);

  if (button)
    return button->data();
  else
    return QVariant();
}

// get button for tab
CQTabBarButton *
CQTabBar::
tabButton(int index) const
{
  if (index >= 0 && index < count()) {
    CQTabBarButton *button = buttons_.at(index);
    assert(button);

    return button;
  }
  else
    return NULL;
}

// get widget for tab
QWidget *
CQTabBar::
tabWidget(int index) const
{
  if (index >= 0 && index < count()) {
    CQTabBarButton *button = buttons_.at(index);
    if (! button) return NULL;

    return button->widget();
  }
  else
    return NULL;
}

// draw tab buttons
void
CQTabBar::
paintEvent(QPaintEvent *)
{
  QStylePainter stylePainter(this);

  //------

  // calculate width and height of region
  int xo = 0;

  if (offset_ > 0) {
    int offset = offset_;

    for (TabButtons::const_iterator p = buttons_.begin();
           offset > 0 && p != buttons_.end(); ++p) {
      CQTabBarButton *button = *p;

      if (! button || ! button->visible()) continue;

      xo += button->width();

      --offset;
    }
  }

  int w = width ();
  int h = height();

  // set tab style
  QStyleOptionTabV2 tabStyle;

  tabStyle.initFrom(this);

  tabStyle.shape = getTabShape();

  int overlap = style()->pixelMetric(QStyle::PM_TabBarBaseOverlap, &tabStyle, this);

  // set tab base style
  QStyleOptionTabBarBaseV2 baseStyle;

  baseStyle.initFrom(this);

  // calculate button geometry and first/last tab buttons
  CQTabBarButton *firstButton = NULL;
  CQTabBarButton *lastButton  = NULL;

  int x = -xo;

  for (TabButtons::const_iterator p = buttons_.begin(); p != buttons_.end(); ++p) {
    CQTabBarButton *button = *p;

    if (! button || ! button->visible()) continue;

    if (firstButton == NULL)
      firstButton = button;
    else
      lastButton = button;

    // calc button width
    int w1 = button->width();

    // calculate and store button rectangle
    QRect r;

    if (isVertical())
      r = QRect(0, x, h_, w1);
    else
      r = QRect(x, 0, w1, h_);

    button->setRect(r);

    // update base line rectangle
    if (button->index() == currentIndex())
      baseStyle.selectedTabRect = r;

    //-----

    x += w1;
  }

  // draw tab base
  if      (position_ == North)
    baseStyle.rect = QRect(0, h_ - overlap, w, overlap);
  else if (position_ == South)
    baseStyle.rect = QRect(0, 0, w, overlap);
  else if (position_ == West)
    baseStyle.rect = QRect(h_ - overlap, 0, overlap, h);
  else if (position_ == East)
    baseStyle.rect = QRect(0, 0, overlap, h);

  baseStyle.shape = getTabShape();

  stylePainter.drawPrimitive(QStyle::PE_FrameTabBarBase, baseStyle);

  //------

  // draw buttons
  x = -xo;

  for (TabButtons::const_iterator p = buttons_.begin(); p != buttons_.end(); ++p) {
    CQTabBarButton *button = *p;

    if (! button || ! button->visible()) continue;

    //----

    // set button style
    tabStyle.initFrom(this);

    tabStyle.state &= ~(QStyle::State_HasFocus | QStyle::State_MouseOver);

    tabStyle.rect = button->rect();

    tabStyle.row = 0;

    if (button->index() == pressIndex_)
      tabStyle.state |= QStyle::State_Sunken;
    else
      tabStyle.state &= ~QStyle::State_Sunken;

    if (button->index() == currentIndex())
      tabStyle.state |=  QStyle::State_Selected;
    else
      tabStyle.state &= ~QStyle::State_Selected;

    if (button->index() == moveIndex_)
      tabStyle.state |=  QStyle::State_MouseOver;
    else
      tabStyle.state &= ~QStyle::State_MouseOver;

    tabStyle.shape = getTabShape();

    if (buttonStyle_ == Qt::ToolButtonTextOnly || buttonStyle_ == Qt::ToolButtonTextBesideIcon)
      tabStyle.text = button->text();

    if (buttonStyle_ == Qt::ToolButtonIconOnly || buttonStyle_ == Qt::ToolButtonTextBesideIcon)
      tabStyle.icon = button->positionIcon(position_);

    tabStyle.iconSize = iconSize();

    if      (button == firstButton)
      tabStyle.position = QStyleOptionTab::Beginning;
    else if (button == lastButton)
      tabStyle.position = QStyleOptionTab::End;
    else
      tabStyle.position = QStyleOptionTab::Middle;

    if (button->pending())
      tabStyle.palette.setColor(QPalette::Button, QColor("#0000FF"));

    // draw button
    stylePainter.drawControl(QStyle::CE_TabBarTab, tabStyle);

    //-----

    x += button->width();
  }

  // update scroll buttons
  lscroll_->setEnabled(offset_ > 0);
  rscroll_->setEnabled(offset_ < clipNum_);
}

// handle resize
void
CQTabBar::
resizeEvent(QResizeEvent *)
{
  updateSizes();
}

// update size of tab area
void
CQTabBar::
updateSizes()
{
  // calculate width and height of region
  QFontMetrics fm(font());

  int iw = iconSize().width();
  int h  = qMax(iw, fm.height()) + TAB_BORDER;

  // remove resize width
  if (isVertical())
    h = qMin(h, width () - RESIZE_WIDTH);
  else
    h = qMin(h, height() - RESIZE_WIDTH);

  int w = 0;

  clipNum_ = 0;

  for (TabButtons::const_iterator p = buttons_.begin(); p != buttons_.end(); ++p) {
    CQTabBarButton *button = *p;

    if (! button || ! button->visible()) continue;

    w += button->width();

    if (isVertical()) {
      if (w > height())
        ++clipNum_;
    }
    else {
      if (w > width())
        ++clipNum_;
    }
  }

  iw_ = iw;
  w_  = w;
  h_  = h;

  //-----

  // update scroll if clipped
  showScrollButtons(clipNum_ > 0);

  if (offset_ > clipNum_)
    offset_ = clipNum_;
}

// update scroll buttons
void
CQTabBar::
showScrollButtons(bool show)
{
  lscroll_->setVisible(show);
  rscroll_->setVisible(show);

  if (show) {
    // position scroll buttons depending in tab position
    if (isVertical()) {
      int xs = iconWidth() + 6;
      int ys = iconWidth();

      int d = h_ - xs;

      lscroll_->setFixedSize(xs, ys);
      rscroll_->setFixedSize(xs, ys);

      lscroll_->move(d, height() - 2*ys);
      rscroll_->move(d, height() -   ys);

      lscroll_->setArrowType(Qt::UpArrow);
      rscroll_->setArrowType(Qt::DownArrow);
    }
    else {
      int xs = iconWidth();
      int ys = iconWidth() + 6;

      int d = h_ - ys;

      lscroll_->setFixedSize(xs, ys);
      rscroll_->setFixedSize(xs, ys);

      lscroll_->move(width() - 2*xs, d);
      rscroll_->move(width() -   xs, d);

      lscroll_->setArrowType(Qt::LeftArrow);
      rscroll_->setArrowType(Qt::RightArrow);
    }
  }
  else
    offset_ = 0;
}

// called when left/bottom scroll is pressed
void
CQTabBar::
lscrollSlot()
{
  // scroll to previous tab (if any)
  --offset_;

  if (offset_ < 0)
    offset_ = 0;

  update();
}

// called when right/top scroll is pressed
void
CQTabBar::
rscrollSlot()
{
  // scroll to next tab (if any)
  ++offset_;

  if (offset_ > clipNum_)
    offset_ = clipNum_;

  update();
}

// handle tool tip event
bool
CQTabBar::
event(QEvent *e)
{
  if (e->type() == QEvent::ToolTip) {
    QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);

    int index = tabAt(helpEvent->pos());

    if (index != -1) {
      CQTabBarButton *button = buttons_.at(index);

      if (button)
        QToolTip::showText(helpEvent->globalPos(), button->toolTip());
    }
    else {
      QToolTip::hideText();

      e->ignore();
    }

    return true;
  }

  return QWidget::event(e);
}

// get preferred size
QSize
CQTabBar::
sizeHint() const
{
  QFontMetrics fm(font());

  int iw = iconSize().width();
  int h  = qMax(iw, fm.height()) + TAB_BORDER + RESIZE_WIDTH;

  int w = 0;

  for (TabButtons::const_iterator p = buttons_.begin(); p != buttons_.end(); ++p) {
    CQTabBarButton *button = *p;

    if (! button || ! button->visible()) continue;

    w += button->width();
  }

  if (isVertical())
    return QSize(h, w);
  else
    return QSize(w, h);
}

// get minimum size
QSize
CQTabBar::
minimumSizeHint() const
{
  QSize s = sizeHint();

  if (isVertical())
    return QSize(s.width(), 0);
  else
    return QSize(0, s.height());
}

// set press point
void
CQTabBar::
setPressPoint(const QPoint &p)
{
  pressed_    = true;
  pressPos_   = p;
  pressIndex_ = tabAt(pressPos_);
}

// handle mouse press
void
CQTabBar::
mousePressEvent(QMouseEvent *e)
{
  // init press state and redraw
  setPressPoint(e->pos());

  update();
}

// handle mouse move (while pressed)
void
CQTabBar::
mouseMoveEvent(QMouseEvent *e)
{
  // update press state and redraw
  if (! pressed_)
    setPressPoint(e->pos());

  // If left button pressed check for drag
  if (e->buttons() & Qt::LeftButton) {
    // check drag distance
    if ((e->pos() - pressPos_).manhattanLength() >= QApplication::startDragDistance()) {
      CQTabBarButton *button = buttons_.at(pressIndex_);

      QIcon icon = (button ? button->icon() : QIcon());

      // initiate drag
      QDrag *drag = new QDrag(this);

      drag->setPixmap(icon.pixmap(iconSize()));

      QMimeData *mimeData = new QMimeData;

      // a crude way to distinguish drags
      mimeData->setData("action", "CQTabBarDrag") ;

      drag->setMimeData(mimeData);

      drag->exec();
    }
  }

  update();
}

// handle mouse release
void
CQTabBar::
mouseReleaseEvent(QMouseEvent *e)
{
  // reset pressed state
  pressed_ = false;

  // check if new tab button is pressed
  pressIndex_ = tabAt(e->pos());

  bool isCurrent = (pressIndex_ != -1 && pressIndex_ == currentIndex());

  if (pressIndex_ != -1) {
    if (! isCurrent)
      setCurrentIndex(pressIndex_); // will send currentChanged signal
    else
      emit currentPressed(pressIndex_);
  }

  // signal tab button pressed
  emit tabPressedSignal(pressIndex_, ! isCurrent);

  // redraw
  update();
}

// handle drag enter event
void
CQTabBar::
dragEnterEvent(QDragEnterEvent *event)
{
  // Only accept if it's our request
  const QMimeData *m = event->mimeData();

  QStringList formats = m->formats();

  if (formats.contains("action") && (m->data("action") == "CQTabBarDrag"))
    event->acceptProposedAction();
}

// handle drop event
void
CQTabBar::
dropEvent(QDropEvent *event)
{
  // from icon at press position to icon at release position
  int fromIndex = tabAt(pressPos_);
  int toIndex   = tabAt(event->pos());

  // skip invalid and do nothing drops
  if (fromIndex < 0 || toIndex < 0 || fromIndex == toIndex)
    return;

  CQTabBarButton *button1 = buttons_[fromIndex];
  CQTabBarButton *button2 = buttons_[toIndex  ];

  buttons_[toIndex  ] = button1;
  buttons_[fromIndex] = button2;

  button1->setIndex(toIndex);
  button2->setIndex(fromIndex);

  if      (fromIndex == currentIndex()) setCurrentIndex(toIndex);
  else if (toIndex   == currentIndex()) setCurrentIndex(fromIndex);

  event->acceptProposedAction();
}

// get tab at specified point
int
CQTabBar::
tabAt(const QPoint &point) const
{
  for (TabButtons::const_iterator p = buttons_.begin(); p != buttons_.end(); ++p) {
    CQTabBarButton *button = *p;

    if (! button || ! button->visible()) continue;

    if (button->rect().contains(point))
      return button->index();
  }

  return -1;
}

// get icon size
QSize
CQTabBar::
iconSize() const
{
  return iconSize_;
}

// set icon size
void
CQTabBar::
setIconSize(const QSize &size)
{
  iconSize_ = size;

  update();
}

// handle context menu request
void
CQTabBar::
contextMenuEvent(QContextMenuEvent *e)
{
  emit showContextMenuSignal(e->globalPos());
}

//! get tab shape for position
QTabBar::Shape
CQTabBar::
getTabShape() const
{
  switch (position_) {
    case North: return QTabBar::RoundedNorth;
    case South: return QTabBar::RoundedSouth;
    case West : return QTabBar::RoundedWest;
    case East : return QTabBar::RoundedEast;
    default   : assert(false); return QTabBar::RoundedNorth;
  }
}

bool
CQTabBar::
isVertical() const
{
  return (position_ == West || position_ == East);
}

//-------

// create tab button
CQTabBarButton::
CQTabBarButton(CQTabBar *bar) :
 bar_(bar), index_(0), text_(), icon_(), positionIcon_(),
 iconPosition_(CQTabBar::North), toolTip_(), w_(0), visible_(true),
 pending_(false), r_()
{
}

// set button text
void
CQTabBarButton::
setText(const QString &text)
{
  text_ = text;
}

// set button icon
void
CQTabBarButton::
setIcon(const QIcon &icon)
{
  icon_ = icon;

  // ensure new icon causes recalc
  if (iconPosition_ != CQTabBar::North && iconPosition_ != CQTabBar::South)
    iconPosition_ = CQTabBar::North;
}

// set button data
void
CQTabBarButton::
setData(const QVariant &data)
{
  data_ = data;
}

// get icon for tab position
const QIcon &
CQTabBarButton::
positionIcon(CQTabBar::Position pos) const
{
  if (pos == CQTabBar::North || pos == CQTabBar::South)
    return icon_;

  if (pos == iconPosition_)
    return positionIcon_;

  iconPosition_ = pos;

  QTransform t;

  QPixmap p = pixmap();

  t.rotate(iconPosition_ == CQTabBar::West ? 90 : -90);

  positionIcon_ = QIcon(p.transformed(t));

  return positionIcon_;
}

// get tab tooltip
const QString &
CQTabBarButton::
toolTip() const
{
  if (toolTip_ != "")
    return toolTip_;
  else
    return text_;
}

// set tab tooltip
void
CQTabBarButton::
setToolTip(const QString &tip)
{
  toolTip_ = tip;
}

// set tab widget
void
CQTabBarButton::
setWidget(QWidget *w)
{
  w_ = w;
}

// set tab visible
void
CQTabBarButton::
setVisible(bool visible)
{
  visible_ = visible;
}

// set tab pending
void
CQTabBarButton::
setPending(bool pending)
{
  pending_ = pending;
}

// set tab rectangle
void
CQTabBarButton::
setRect(const QRect &r)
{
  r_ = r;
}

// get tab icon pixmap
QPixmap
CQTabBarButton::
pixmap() const
{
  return icon_.pixmap(bar_->iconSize());
}

// get button width depending on button style
int
CQTabBarButton::
width() const
{
  QFontMetrics fm(bar_->font());

  //------

  Qt::ToolButtonStyle buttonStyle = bar_->buttonStyle();

  int w = 0;

  if      (buttonStyle == Qt::ToolButtonTextOnly)
    w = fm.width(text()) + 24;
  else if (buttonStyle == Qt::ToolButtonIconOnly)
    w = bar_->iconWidth() + 24;
  else
    w = bar_->iconWidth() + fm.width(text()) + 32;

  return w;
}

//---------

CQTabBarScrollButton::
CQTabBarScrollButton(CQTabBar *bar, const char *name) :
 QToolButton(bar)
{
  setObjectName(name);

  setAutoRepeat(true);

  setFocusPolicy(Qt::NoFocus);
}