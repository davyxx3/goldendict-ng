#include "dictionarybar.hh"
#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QContextMenuEvent>
#include <QProcess>
#include "gddebug.hh"


using std::vector;

DictionaryBar::DictionaryBar( QWidget * parent,
                              Config::Events & events, QString const & _editDictionaryCommand, unsigned short const & maxDictionaryRefsInContextMenu_ ):
  QToolBar( tr( "&Dictionary Bar" ), parent ),
  mutedDictionaries( 0 ),
  configEvents( events ),
  editDictionaryCommand( _editDictionaryCommand ),
  maxDictionaryRefsInContextMenu(maxDictionaryRefsInContextMenu_)
{
  setObjectName( "dictionaryBar" );

  maxDictionaryRefsAction = new QAction(  QIcon(":/icons/expand_opt.png"), tr( "Extended menu with all dictionaries..." ), this );

  connect( &events, &Config::Events::mutedDictionariesChanged, this, &DictionaryBar::mutedDictionariesChanged );

  connect( this, &QToolBar::actionTriggered, this, &DictionaryBar::actionWasTriggered );
}

static QString elideDictName( QString const & name )
{
  // Some names are way too long -- we insert an ellipsis in the middle of those

  int const maxSize = 33;

  if ( name.size() <= maxSize )
    return name;

  int const pieceSize = maxSize / 2 - 1;

  return name.left( pieceSize ) + QChar( 0x2026 ) + name.right( pieceSize );
}

void DictionaryBar::setDictionaries( vector< sptr< Dictionary::Class > >
                                     const & dictionaries )
{
  setUpdatesEnabled( false );

  allDictionaries.clear();
  allDictionaries = dictionaries;

  clear();
  dictActions.clear();

  for( unsigned x = 0; x < dictionaries.size(); ++x )
  {
    QIcon icon = dictionaries[ x ]->getNativeIcon();

    QString dictName = QString::fromUtf8( dictionaries[ x ]->
                                            getName().c_str() );

    QAction * action = addAction( icon, elideDictName( dictName ) );

    action->setToolTip( dictName ); // Tooltip need not be shortened

    QString id = QString::fromStdString( dictionaries[ x ]->getId() );

    action->setData( id );

    action->setCheckable( true );

    action->setChecked( mutedDictionaries ? !mutedDictionaries->contains( id ) : true );

    dictActions.append( action );
  }

  setDictionaryIconSize( 21 );

  setUpdatesEnabled( true );
}

void DictionaryBar::setDictionaryIconSize( int extent )
{
  setIconSize( QSize(  extent, extent  ) );
}

void DictionaryBar::contextMenuEvent( QContextMenuEvent * event )
{
  showContextMenu( event );
}

void DictionaryBar::showContextMenu( QContextMenuEvent * event, bool extended )
{
  QMenu menu( this );

  QAction * editAction =
      menu.addAction( QIcon( ":/icons/bookcase.svg" ), tr( "Edit this group" ) );

  QAction * infoAction = NULL;
  QAction * headwordsAction = NULL;
  QAction * editDictAction = NULL;
  QAction * openDictFolderAction = NULL;
  QString dictFilename;

  QAction * dictAction = actionAt( event->x(), event->y() );
  if( dictAction )
  {
    Dictionary::Class *pDict = NULL;
    QString id = dictAction->data().toString();
    for( unsigned i = 0; i < allDictionaries.size(); i++ )
    {
      if( id.compare( allDictionaries[ i ]->getId().c_str() ) == 0 )
      {
        pDict = allDictionaries[ i ].get();
        break;
      }
    }

    if( pDict )
    {
      infoAction =  menu.addAction( tr( "Dictionary info" ) );

      if( pDict->isLocalDictionary() )
      {
        if( pDict->getWordCount() > 0 )
          headwordsAction = menu.addAction( tr( "Dictionary headwords" ) );

        openDictFolderAction = menu.addAction( tr( "Open dictionary folder" ) );

        if( !editDictionaryCommand.isEmpty() )
        {
          if( !pDict->getMainFilename().isEmpty() )
          {
            dictFilename = pDict->getMainFilename();
            editDictAction = menu.addAction( tr( "Edit dictionary" ) );
          }
        }
      }
    }
  }

  if ( !dictActions.empty() )
    menu.addSeparator();

  unsigned refsAdded = 0;

  for( QList< QAction * >::iterator i = dictActions.begin();
       i != dictActions.end(); ++i )
  {

    // Enough! Or the menu would become too large.
    if ( refsAdded++ >= maxDictionaryRefsInContextMenu && !extended )
    {
      menu.addSeparator();
      menu.addAction( maxDictionaryRefsAction );
      break;
    }

    // We need new action, since the one we have has text elided
    QAction * action = menu.addAction( (*i)->icon(), (*i)->toolTip() );

    action->setCheckable( true );
    action->setChecked( (*i)->isChecked() );
    action->setData( QVariant::fromValue( (void *)*i ) );
    // Force "icon in menu" on all platforms, for
    // usability reasons.
    action->setIconVisibleInMenu( true );
  }

  connect( this, &DictionaryBar::closePopupMenu, &menu, &QWidget::close );

  QAction * result = menu.exec( event->globalPos() );

  if( result && result == infoAction )
  {
    QString id = dictAction->data().toString();
    emit showDictionaryInfo( id );
    return;
  }

  if ( result && result == headwordsAction ) {
    std::string id = dictAction->data().toString().toStdString();
    // TODO: use `Dictionary::class*` instead of `QString id` at action->setData to remove all similar `for` loops
    for ( const auto & dict : allDictionaries ) {
      if ( id == dict->getId() ) {
        emit showDictionaryHeadwords( dict.get() );
        break;
      }
    }
    return;
  }

  if( result && result == openDictFolderAction )
  {
    QString id = dictAction->data().toString();
    emit openDictionaryFolder( id );
    return;
  }

  if( result && result == editDictAction )
  {
    QString command( editDictionaryCommand );
    command.replace( "%GDDICT%", "\"" + dictFilename + "\"" );
    if( !QProcess::startDetached( command ,QStringList()) )
      QApplication::beep();
  }

  if( result && result == maxDictionaryRefsAction )
  {
    showContextMenu( event, true );
  }

  if ( result == editAction )
    emit editGroupRequested();
  else
  if ( result && result->data().value< void * >() )
    ( ( QAction * )( result->data().value< void * >() ) )->trigger();

  event->accept();
}

void DictionaryBar::mutedDictionariesChanged()
{
  //GD_DPRINTF( "Muted dictionaries changed\n" );

  if( !mutedDictionaries )
    return;

  // Update actions

  setUpdatesEnabled( false );

  for( QList< QAction * >::iterator i = dictActions.begin();
       i != dictActions.end(); ++i )
  {
    bool isUnmuted = !mutedDictionaries->contains( (*i)->data().toString() );

    if ( isUnmuted != (*i)->isChecked() )
      (*i)->setChecked( isUnmuted );
  }

  setUpdatesEnabled( true );
}

void DictionaryBar::actionWasTriggered( QAction * action )
{
  if( !mutedDictionaries )
    return;

  QString id = action->data().toString();

  if ( id.isEmpty() )
    return; // Some weird action, not our button

  if ( QApplication::keyboardModifiers() &
         ( Qt::ControlModifier | Qt::ShiftModifier ) )
  {
    // Solo mode -- either use the dictionary exclusively, or toggle
    // back all dictionaries if we do that already.

    // Are we solo already?

    bool isSolo = true;

    // For solo, all dictionaries must be unchecked, since we're handling
    // the result of the dictionary being (un)checked, and in case we were
    // in solo, now we would end up with no dictionaries being checked at all.
    for( QList< QAction * >::iterator i = dictActions.begin();
         i != dictActions.end(); ++i )
    {
      if ( (*i)->isChecked() )
      {
        isSolo = false;
        break;
      }
    }

    if ( isSolo )
    {
      // Restore or clear all the dictionaries
      if ( QApplication::keyboardModifiers() & Qt::ShiftModifier )
        *mutedDictionaries = storedMutedSet;
      else
      {
        for( QList< QAction * >::iterator i = dictActions.begin();
             i != dictActions.end(); ++i )
          mutedDictionaries->remove( (*i)->data().toString() );
      }
      storedMutedSet.clear();
    }
    else
    {
      // Save dictionaries state
      storedMutedSet = *mutedDictionaries;

      // Make dictionary solo
      for( QList< QAction * >::iterator i = dictActions.begin();
           i != dictActions.end(); ++i )
      {
        QString dictId = (*i)->data().toString();

        if ( dictId == id )
          mutedDictionaries->remove( dictId );
        else
          mutedDictionaries->insert( dictId );
      }
    }  
    configEvents.signalMutedDictionariesChanged();
  }
  else
  {
    // Normal mode

    storedMutedSet.clear();

    if ( action->isChecked() )
    {
      // Unmute the dictionary

      if ( mutedDictionaries->contains( id ) )
      {
        mutedDictionaries->remove( id );
        configEvents.signalMutedDictionariesChanged();
      }
    }
    else
    {
      // Mute the dictionary

      if ( !mutedDictionaries->contains( id ) )
      {
        mutedDictionaries->insert( id );
        configEvents.signalMutedDictionariesChanged();
      }
    }
  }
}

void DictionaryBar::dictsPaneClicked( const QString & id )
{
  if ( !isVisible() )
    return;

  for( QList< QAction * >::iterator i = dictActions.begin();
       i != dictActions.end(); ++i )
  {
    QString dictId = (*i)->data().toString();
    if ( dictId == id )
    {
      (*i)->activate( QAction::Trigger );
      break;
    }
  }
}
