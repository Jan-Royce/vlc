/*****************************************************************************
 * extended_panels.cpp : Extended controls panels
 ****************************************************************************
 * Copyright (C) 2006-2007 the VideoLAN team
 * $Id$
 *
 * Authors: Clément Stenac <zorglub@videolan.org>
 *          Antoine Cellerier <dionoea .t videolan d@t org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include <QLabel>
#include <QVariant>
#include <QString>
#include <QFont>
#include <QGridLayout>
#include <QSignalMapper>

#include "components/extended_panels.hpp"
#include "dialogs/preferences.hpp"
#include "dialogs_provider.hpp"
#include "qt4.hpp"

#include "../../audio_filter/equalizer_presets.h"
#include <vlc_aout.h>
#include <vlc_intf_strings.h>
#include <vlc_vout.h>
#include <vlc_osd.h>

#include <iostream>

#if 0
class ConfClickHandler : public QObject
{
public:
    ConfClickHandler( intf_thread_t *_p_intf, ExtVideo *_e ) : QObject (_e) {
        e = _e; p_intf = _p_intf;
    }
    virtual ~ConfClickHandler() {}
    bool eventFilter( QObject *obj, QEvent *evt )
    {
        if( evt->type() == QEvent::MouseButtonPress )
        {
            e->gotoConf( obj );
            return true;
        }
        return false;
    }
private:
    ExtVideo* e;
    intf_thread_t *p_intf;
};
#endif

QString ModuleFromWidgetName( QObject *obj )
{
    return obj->objectName().replace("Enable","");
}

QString OptionFromWidgetName( QObject *obj )
{
    /* Gruik ? ... nah */
    QString option = obj->objectName().replace( "Slider", "" )
                                      .replace( "Combo" , "" )
                                      .replace( "Dial"  , "" )
                                      .replace( "Check" , "" )
                                      .replace( "Spin"  , "" )
                                      .replace( "Text"  , "" );
    for( char a = 'A'; a <= 'Z'; a++ )
    {
        option = option.replace( QString( a ),
                                 QString( '-' ) + QString( a + 'a' - 'A' ) );
    }
    return option;
}

ExtVideo::ExtVideo( intf_thread_t *_p_intf, QWidget *_parent ) :
                           QWidget( _parent ) , p_intf( _p_intf )
{
    ui.setupUi( this );

#define SETUP_VFILTER( widget ) \
    { \
        vlc_object_t *p_obj = (vlc_object_t *) \
            vlc_object_find_name( p_intf->p_libvlc, \
                                  #widget, \
                                  FIND_CHILD ); \
        QCheckBox *checkbox = qobject_cast<QCheckBox*>(ui.widget##Enable); \
        QGroupBox *groupbox = qobject_cast<QGroupBox*>(ui.widget##Enable); \
        if( p_obj ) \
        { \
            vlc_object_release( p_obj ); \
            if( checkbox ) checkbox->setCheckState( Qt::Checked ); \
            else groupbox->setChecked( true ); \
        } \
        else \
        { \
            if( checkbox ) checkbox->setCheckState( Qt::Unchecked ); \
            else groupbox->setChecked( false ); \
        } \
    } \
    CONNECT( ui.widget##Enable, clicked(), this, updateFilters() );
#define SETUP_VFILTER_OPTION( widget, signal ) \
    setWidgetValue( ui.widget ); \
    CONNECT( ui.widget, signal, this, updateFilterOptions() );

    SETUP_VFILTER( adjust )
    SETUP_VFILTER_OPTION( hueSlider, valueChanged(int) )
    SETUP_VFILTER_OPTION( contrastSlider, valueChanged(int) )
    SETUP_VFILTER_OPTION( brightnessSlider, valueChanged(int) )
    SETUP_VFILTER_OPTION( saturationSlider, valueChanged(int) )
    SETUP_VFILTER_OPTION( gammaSlider, valueChanged(int) )
    SETUP_VFILTER_OPTION( brightnessThresholdCheck, stateChanged(int) )

    SETUP_VFILTER( extract )
    SETUP_VFILTER_OPTION( extractComponentText, textChanged(QString) )

    SETUP_VFILTER( colorthres )
    SETUP_VFILTER_OPTION( colorthresColorText, textChanged(QString) )
    SETUP_VFILTER_OPTION( colorthresSaturationthresSlider, valueChanged(int) )
    SETUP_VFILTER_OPTION( colorthresSimilaritythresSlider, valueChanged(int) )

    SETUP_VFILTER( invert )

    SETUP_VFILTER( gradient )
    SETUP_VFILTER_OPTION( gradientModeCombo, currentIndexChanged(QString) )
    SETUP_VFILTER_OPTION( gradientTypeCheck, stateChanged(int) )
    SETUP_VFILTER_OPTION( gradientCartoonCheck, stateChanged(int) )

    SETUP_VFILTER( blur )
    SETUP_VFILTER_OPTION( blurFactorSlider, valueChanged(int) )

    SETUP_VFILTER( motiondetect )

    SETUP_VFILTER( noise )

    SETUP_VFILTER( psychedelic )

    SETUP_VFILTER( sharpen )
    SETUP_VFILTER_OPTION( sharpenSigmaSlider, valueChanged(int) )

    SETUP_VFILTER( ripple )

    SETUP_VFILTER( wave )

    SETUP_VFILTER( transform )
    SETUP_VFILTER_OPTION( transformTypeCombo, currentIndexChanged(QString) )

    SETUP_VFILTER( rotate )
    SETUP_VFILTER_OPTION( rotateAngleDial, valueChanged(int) )
    ui.rotateAngleDial->setWrapping( true );
    ui.rotateAngleDial->setNotchesVisible( true );

    SETUP_VFILTER( puzzle )
    SETUP_VFILTER_OPTION( puzzleRowsSpin, valueChanged(int) )
    SETUP_VFILTER_OPTION( puzzleColsSpin, valueChanged(int) )
    SETUP_VFILTER_OPTION( puzzleBlackSlotCheck, stateChanged(int) )

    SETUP_VFILTER( magnify )

    SETUP_VFILTER( clone )
    SETUP_VFILTER_OPTION( cloneCountSpin, valueChanged(int) )

    SETUP_VFILTER( wall )
    SETUP_VFILTER_OPTION( wallRowsSpin, valueChanged(int) )
    SETUP_VFILTER_OPTION( wallColsSpin, valueChanged(int) )

    SETUP_VFILTER( erase )
    SETUP_VFILTER_OPTION( eraseMaskText, editingFinished() )
    SETUP_VFILTER_OPTION( eraseYSpin, valueChanged(int) )
    SETUP_VFILTER_OPTION( eraseXSpin, valueChanged(int) )

    SETUP_VFILTER( marq )
    SETUP_VFILTER_OPTION( marqMarqueeText, textChanged(QString) )
    SETUP_VFILTER_OPTION( marqPositionCombo, currentIndexChanged(QString) )

    SETUP_VFILTER( logo )
    SETUP_VFILTER_OPTION( logoFileText, editingFinished() )
    SETUP_VFILTER_OPTION( logoYSpin, valueChanged(int) )
    SETUP_VFILTER_OPTION( logoXSpin, valueChanged(int) )
    SETUP_VFILTER_OPTION( logoTransparencySlider, valueChanged(int) )

#undef SETUP_VFILTER
#undef SETUP_VFILTER_OPTION
}

ExtVideo::~ExtVideo()
{
}

void ExtVideo::ChangeVFiltersString( char *psz_name, vlc_bool_t b_add )
{
    vout_thread_t *p_vout;
    char *psz_parser, *psz_string;

    char *psz_filter_type;
    vlc_object_t *p_obj = (vlc_object_t *)
        vlc_object_find_name( p_intf->p_libvlc_global, psz_name, FIND_CHILD );
    if( !p_obj )
    {
        msg_Err( p_intf, "Unable to find filter module." );
        return;
    }

    if( !strcmp( ((module_t*)p_obj)->psz_capability, "video filter2" ) )
    {
        psz_filter_type = "video-filter";
    }
    else if( !strcmp( ((module_t*)p_obj)->psz_capability, "video filter" ) )
    {
        psz_filter_type = "vout-filter";
    }
    else if( !strcmp( ((module_t*)p_obj)->psz_capability, "sub filter" ) )
    {
        psz_filter_type = "sub-filter";
    }
    else
    {
        vlc_object_release( p_obj );
        msg_Err( p_intf, "Unknown video filter type." );
        return;
    }
    vlc_object_release( p_obj );

    psz_string = config_GetPsz( p_intf, psz_filter_type );

    if( !psz_string ) psz_string = strdup("");

    psz_parser = strstr( psz_string, psz_name );

    if( b_add )
    {
        if( !psz_parser )
        {
            psz_parser = psz_string;
            asprintf( &psz_string, (*psz_string) ? "%s:%s" : "%s%s",
                            psz_string, psz_name );
            free( psz_parser );
        }
        else
        {
            return;
        }
    }
    else
    {
        if( psz_parser )
        {
            if( *(psz_parser + strlen(psz_name)) == ':' )
            {
                memmove( psz_parser, psz_parser + strlen(psz_name) + 1,
                         strlen(psz_parser + strlen(psz_name) + 1 ) + 1 );
            }
            else
            {
                *psz_parser = '\0';
            }

            /* Remove trailing : : */
            if( strlen( psz_string ) > 0 &&
                *( psz_string + strlen( psz_string ) -1 ) == ':' )
            {
                *( psz_string + strlen( psz_string ) -1 ) = '\0';
            }
        }
        else
        {
            free( psz_string );
            return;
        }
    }
    /* Vout is not kept, so put that in the config */
    config_PutPsz( p_intf, psz_filter_type, psz_string );
    if( !strcmp( psz_filter_type, "video-filter" ) )
        ui.videoFilterText->setText( psz_string );
    else if( !strcmp( psz_filter_type, "vout-filter" ) )
        ui.voutFilterText->setText( psz_string );
    else if( !strcmp( psz_filter_type, "sub-filter" ) )
        ui.subpictureFilterText->setText( psz_string );

    /* Try to set on the fly */
    p_vout = (vout_thread_t *)vlc_object_find( p_intf, VLC_OBJECT_VOUT,
                                              FIND_ANYWHERE );
    if( p_vout )
    {
        if( !strcmp( psz_filter_type, "sub-filter" ) )
            var_SetString( p_vout->p_spu, psz_filter_type, psz_string );
        else
            var_SetString( p_vout, psz_filter_type, psz_string );
        vlc_object_release( p_vout );
    }

    free( psz_string );
}

void ExtVideo::updateFilters()
{
    QString module = ModuleFromWidgetName( sender() );
    //std::cout << "Module name: " << module.toStdString() << std::endl;

    QCheckBox *checkbox = qobject_cast<QCheckBox*>(sender());
    QGroupBox *groupbox = qobject_cast<QGroupBox*>(sender());

    ChangeVFiltersString( qtu(module),
                          checkbox ? checkbox->isChecked()
                                   : groupbox->isChecked() );
}

void ExtVideo::setWidgetValue( QObject *widget )
{
    QString module = ModuleFromWidgetName( widget->parent() );
    //std::cout << "Module name: " << module.toStdString() << std::endl;
    QString option = OptionFromWidgetName( widget );
    //std::cout << "Option name: " << option.toStdString() << std::endl;

    vlc_object_t *p_obj = (vlc_object_t *)
        vlc_object_find_name( p_intf->p_libvlc,
                              module.toStdString().c_str(),
                              FIND_CHILD );
    int i_type;
    vlc_value_t val;

    if( !p_obj )
    {
        msg_Dbg( p_intf,
                 "Module instance %s not found, looking in config values.",
                 module.toStdString().c_str() );
        i_type = config_GetType( p_intf, option.toStdString().c_str() ) & 0xf0;
        switch( i_type )
        {
            case VLC_VAR_INTEGER:
            case VLC_VAR_BOOL:
                val.i_int = config_GetInt( p_intf, option.toStdString().c_str() );
                break;
            case VLC_VAR_FLOAT:
                val.f_float = config_GetFloat( p_intf, option.toStdString().c_str() );
                break;
            case VLC_VAR_STRING:
                val.psz_string = config_GetPsz( p_intf, option.toStdString().c_str() );
                break;
        }
    }
    else
    {
        i_type = var_Type( p_obj, option.toStdString().c_str() ) & 0xf0;
        var_Get( p_obj, option.toStdString().c_str(), &val );
        vlc_object_release( p_obj );
    }

    /* Try to cast to all the widgets we're likely to encounter. Only
     * one of the casts is expected to work. */
    QSlider        *slider        = qobject_cast<QSlider*>       (widget);
    QCheckBox      *checkbox      = qobject_cast<QCheckBox*>     (widget);
    QSpinBox       *spinbox       = qobject_cast<QSpinBox*>      (widget);
    QDoubleSpinBox *doublespinbox = qobject_cast<QDoubleSpinBox*>(widget);
    QDial          *dial          = qobject_cast<QDial*>         (widget);
    QLineEdit      *lineedit      = qobject_cast<QLineEdit*>     (widget);
    QComboBox      *combobox      = qobject_cast<QComboBox*>     (widget);

    if( i_type == VLC_VAR_INTEGER || i_type == VLC_VAR_BOOL )
    {
        int i_int = 0;
        if( slider )        slider->setValue( val.i_int );
        else if( checkbox ) checkbox->setCheckState( val.i_int? Qt::Checked
                                                              : Qt::Unchecked );
        else if( spinbox )  spinbox->setValue( val.i_int );
        else if( dial )     dial->setValue( (540-val.i_int)%360 );
        else if( lineedit )
        {
            char str[30];
            sprintf( str, "%06X", val.i_int );
            lineedit->setText( str );
        }
        else msg_Warn( p_intf, "Oops %s %s %d", __FILE__, __func__, __LINE__ );
    }
    else if( i_type == VLC_VAR_FLOAT )
    {
        double f_float = 0;
        if( slider ) slider->setValue( (int)(val.f_float*(double)slider->tickInterval())); /* hack alert! */
        else if( doublespinbox ) doublespinbox->setValue(val.f_float);
        else msg_Warn( p_intf, "Oops %s %s %d", __FILE__, __func__, __LINE__ );
    }
    else if( i_type == VLC_VAR_STRING )
    {
        const char *psz_string = NULL;
        if( lineedit ) lineedit->setText( qfu(val.psz_string) );
        else msg_Warn( p_intf, "Oops %s %s %d", __FILE__, __func__, __LINE__ );
        free( val.psz_string );
    }
    else
        msg_Err( p_intf,
                 "Module %s's %s variable is of an unsupported type (%d)",
                 module.toStdString().c_str(),
                 option.toStdString().c_str(),
                 i_type );
}

void ExtVideo::updateFilterOptions()
{
    QString module = ModuleFromWidgetName( sender()->parent() );
    //std::cout << "Module name: " << module.toStdString() << std::endl;
    QString option = OptionFromWidgetName( sender() );
    //std::cout << "Option name: " << option.toStdString() << std::endl;

    vlc_object_t *p_obj = (vlc_object_t *)
        vlc_object_find_name( p_intf->p_libvlc,
                              module.toStdString().c_str(),
                              FIND_CHILD );
    if( !p_obj )
    {
        msg_Err( p_intf, "Module %s not found.", module.toStdString().c_str() );
        return;
    }

    int i_type = var_Type( p_obj, option.toStdString().c_str() );
    if( !( i_type & VLC_VAR_ISCOMMAND ) )
    {
        vlc_object_release( p_obj );
        msg_Err( p_intf, "Module %s's %s variable isn't a command.",
                 module.toStdString().c_str(),
                 option.toStdString().c_str() );
        return;
    }

    /* Try to cast to all the widgets we're likely to encounter. Only
     * one of the casts is expected to work. */
    QSlider        *slider        = qobject_cast<QSlider*>       (sender());
    QCheckBox      *checkbox      = qobject_cast<QCheckBox*>     (sender());
    QSpinBox       *spinbox       = qobject_cast<QSpinBox*>      (sender());
    QDoubleSpinBox *doublespinbox = qobject_cast<QDoubleSpinBox*>(sender());
    QDial          *dial          = qobject_cast<QDial*>         (sender());
    QLineEdit      *lineedit      = qobject_cast<QLineEdit*>     (sender());
    QComboBox      *combobox      = qobject_cast<QComboBox*>     (sender());

    i_type &= 0xf0;
    if( i_type == VLC_VAR_INTEGER || i_type == VLC_VAR_BOOL )
    {
        int i_int = 0;
        if( slider )        i_int = slider->value();
        else if( checkbox ) i_int = checkbox->checkState() == Qt::Checked;
        else if( spinbox )  i_int = spinbox->value();
        else if( dial )     i_int = (540-dial->value())%360;
        else if( lineedit ) i_int = lineedit->text().toInt(NULL,16);
        else msg_Warn( p_intf, "Oops %s %s %d", __FILE__, __func__, __LINE__ );
        config_PutInt( p_intf, option.toStdString().c_str(), i_int );
        if( i_type == VLC_VAR_INTEGER )
            var_SetInteger( p_obj, option.toStdString().c_str(), i_int );
        else
            var_SetBool( p_obj, option.toStdString().c_str(), i_int );
    }
    else if( i_type == VLC_VAR_FLOAT )
    {
        double f_float = 0;
        if( slider )             f_float = (double)slider->value()
                                         / (double)slider->tickInterval(); /* hack alert! */
        else if( doublespinbox ) f_float = doublespinbox->value();
        else if( lineedit ) f_float = lineedit->text().toDouble();
        else msg_Warn( p_intf, "Oops %s %s %d", __FILE__, __func__, __LINE__ );
        config_PutFloat( p_intf, option.toStdString().c_str(), f_float );
        var_SetFloat( p_obj, option.toStdString().c_str(), f_float );
    }
    else if( i_type == VLC_VAR_STRING )
    {
        char *psz_string = NULL;
        if( lineedit ) psz_string = qtu(lineedit->text());
        else msg_Warn( p_intf, "Oops %s %s %d", __FILE__, __func__, __LINE__ );
        config_PutPsz( p_intf, option.toStdString().c_str(), psz_string );
        var_SetString( p_obj, option.toStdString().c_str(), psz_string );
    }
    else
        msg_Err( p_intf,
                 "Module %s's %s variable is of an unsupported type (%d)",
                 module.toStdString().c_str(),
                 option.toStdString().c_str(),
                 i_type );

    vlc_object_release( p_obj );
}

#if 0
void ExtVideo::gotoConf( QObject* src )
{
#define SHOWCONF(module) \
    if( src->objectName().contains(module) ) \
    { \
        PrefsDialog::getInstance( p_intf )->showModulePrefs( module ); \
        return; \
    }
    SHOWCONF( "clone" );
    SHOWCONF( "magnify" );
    SHOWCONF( "wave" );
    SHOWCONF( "ripple" );
    SHOWCONF( "invert" );
    SHOWCONF( "puzzle" );
    SHOWCONF( "wall" );
    SHOWCONF( "gradient" );
    SHOWCONF( "colorthres" )
}
#endif

/**********************************************************************
 * Equalizer
 **********************************************************************/

static const QString band_frequencies[] =
{
    "   60Hz  ", " 170 Hz " , " 310 Hz ", " 600 Hz ", "  1 kHz  ",
    "  3 kHz  " , "  6 kHz ", " 12 kHz ", " 14 kHz ", " 16 kHz "
};

Equalizer::Equalizer( intf_thread_t *_p_intf, QWidget *_parent ) :
                            QWidget( _parent ) , p_intf( _p_intf )
{
    QFont smallFont = QApplication::font( static_cast<QWidget*>(0) );
    smallFont.setPointSize( smallFont.pointSize() - 3 );

    ui.setupUi( this );

    ui.preampLabel->setFont( smallFont );
    ui.preampSlider->setMaximum( 400 );
    for( int i = 0 ; i < NB_PRESETS ; i ++ )
    {
        ui.presetsCombo->addItem( qtr( preset_list_text[i] ),
                                  QVariant( i ) );
    }
    CONNECT( ui.presetsCombo, activated( int ), this, setPreset( int ) );

    BUTTONACT( ui.enableCheck, enable() );
    BUTTONACT( ui.eq2PassCheck, set2Pass() );

    CONNECT( ui.preampSlider, valueChanged(int), this, setPreamp() );

    QGridLayout *grid = new QGridLayout( ui.frame );
    grid->setMargin( 0 );
    for( int i = 0 ; i < BANDS ; i++ )
    {
        bands[i] = new QSlider( Qt::Vertical );
        bands[i]->setMaximum( 400 );
        bands[i]->setValue( 200 );
        CONNECT( bands[i], valueChanged(int), this, setBand() );
        band_texts[i] = new QLabel( band_frequencies[i] + "\n0.0dB" );
        band_texts[i]->setFont( smallFont );
        grid->addWidget( bands[i], 0, i );
        grid->addWidget( band_texts[i], 1, i );
    }

    /* Write down initial values */
    aout_instance_t *p_aout = (aout_instance_t *)vlc_object_find(p_intf,
                                    VLC_OBJECT_AOUT, FIND_ANYWHERE);
    char *psz_af = NULL;
    char *psz_bands;
    float f_preamp;
    if( p_aout )
    {
        psz_af = var_GetString( p_aout, "audio-filter" );
        if( var_GetBool( p_aout, "equalizer-2pass" ) )
            ui.eq2PassCheck->setChecked( true );
        psz_bands = var_GetString( p_aout, "equalizer-bands" );
        f_preamp = var_GetFloat( p_aout, "equalizer-preamp" );
        vlc_object_release( p_aout );
    }
    else
    {
        psz_af = config_GetPsz( p_intf, "audio-filter" );
        if( config_GetInt( p_intf, "equalizer-2pass" ) )
            ui.eq2PassCheck->setChecked( true );
        psz_bands = config_GetPsz( p_intf, "equalizer-bands" );
        f_preamp = config_GetFloat( p_intf, "equalizer-preamp" );
    }
    if( psz_af && strstr( psz_af, "equalizer" ) != NULL )
        ui.enableCheck->setChecked( true );
    enable( ui.enableCheck->isChecked() );

    setValues( psz_bands, f_preamp );
}

Equalizer::~Equalizer()
{
}

void Equalizer::enable()
{
    bool en = ui.enableCheck->isChecked();
    aout_EnableFilter( VLC_OBJECT( p_intf ), "equalizer",
                       en ? VLC_TRUE : VLC_FALSE );
    enable( en );
}

void Equalizer::enable( bool en )
{
    ui.eq2PassCheck->setEnabled( en );
    ui.preampLabel->setEnabled( en );
    ui.preampSlider->setEnabled( en  );
    for( int i = 0 ; i< BANDS; i++ )
    {
        bands[i]->setEnabled( en ); band_texts[i]->setEnabled( en );
    }
}

void Equalizer::set2Pass()
{
    aout_instance_t *p_aout= (aout_instance_t *)vlc_object_find(p_intf,
                                 VLC_OBJECT_AOUT, FIND_ANYWHERE);
    vlc_bool_t b_2p = ui.eq2PassCheck->isChecked();

    if( p_aout == NULL )
        config_PutInt( p_intf, "equalizer-2pass", b_2p );
    else
    {
        var_SetBool( p_aout, "equalizer-2pass", b_2p );
        config_PutInt( p_intf, "equalizer-2pass", b_2p );
        for( int i = 0; i < p_aout->i_nb_inputs; i++ )
        {
            p_aout->pp_inputs[i]->b_restart = VLC_TRUE;
        }
        vlc_object_release( p_aout );
    }
}

void Equalizer::setPreamp()
{
    float f= (float)(  ui.preampSlider->value() ) /10 - 20;
    char psz_val[5];
    aout_instance_t *p_aout= (aout_instance_t *)vlc_object_find(p_intf,
                                       VLC_OBJECT_AOUT, FIND_ANYWHERE);

    sprintf( psz_val, "%.1f", f );
    ui.preampLabel->setText( qtr("Preamp\n") + psz_val + qtr("dB") );
    if( p_aout )
    {
        delCallbacks( p_aout );
        var_SetFloat( p_aout, "equalizer-preamp", f );
        addCallbacks( p_aout );
        vlc_object_release( p_aout );
    }
    config_PutFloat( p_intf, "equalizer-preamp", f );
}

void Equalizer::setBand()
{
    char psz_values[102]; memset( psz_values, 0, 102 );

    /**\todo smoothing */

    for( int i = 0 ; i< BANDS ; i++ )
    {
        char psz_val[5];
        float f_val = (float)(  bands[i]->value() ) / 10 - 20 ;
        sprintf( psz_values, "%s %f", psz_values, f_val );
        sprintf( psz_val, "% 5.1f", f_val );
        band_texts[i]->setText( band_frequencies[i] + "\n" + psz_val + "dB" );
    }
    aout_instance_t *p_aout= (aout_instance_t *)vlc_object_find(p_intf,
                                          VLC_OBJECT_AOUT, FIND_ANYWHERE);
    if( p_aout )
    {
        delCallbacks( p_aout );
        var_SetString( p_aout, "equalizer-bands", psz_values );
        addCallbacks( p_aout );
        vlc_object_release( p_aout );
    }
}
void Equalizer::setValues( char *psz_bands, float f_preamp )
{
    char *p = psz_bands;
    if ( p )
    {
        for( int i = 0; i < 10; i++ )
        {
            char psz_val[5];
            float f = strtof( p, &p );
            int  i_val= (int)( ( f + 20 ) * 10 );
            bands[i]->setValue(  i_val );
            sprintf( psz_val, "% 5.1f", f );
            band_texts[i]->setText( band_frequencies[i] + "\n" + psz_val +
                                    "dB" );
            if( p == NULL || *p == '\0' ) break;
            p++;
            if( *p == '\0' )  break;
        }
    }
    char psz_val[5];
    int i_val = (int)( ( f_preamp + 20 ) * 10 );
    sprintf( psz_val, "%.1f", f_preamp );
    ui.preampSlider->setValue( i_val );
    ui.preampLabel->setText( qtr("Preamp\n") + psz_val + qtr("dB") );
}

void Equalizer::setPreset( int preset )
{
    aout_instance_t *p_aout= (aout_instance_t *)vlc_object_find(p_intf,
                                                VLC_OBJECT_AOUT, FIND_ANYWHERE);

    char psz_values[102]; memset( psz_values, 0, 102 );
    for( int i = 0 ; i< 10 ;i++ )
        sprintf( psz_values, "%s %.1f", psz_values,
                                        eqz_preset_10b[preset]->f_amp[i] );

    if( p_aout )
    {
        delCallbacks( p_aout );
        var_SetString( p_aout, "equalizer-bands", psz_values );
        var_SetFloat( p_aout, "equalizer-preamp",
                      eqz_preset_10b[preset]->f_preamp );
        addCallbacks( p_aout );
        vlc_object_release( p_aout );
    }
    config_PutPsz( p_intf, "equalizer-bands", psz_values );
    config_PutFloat( p_intf, "equalizer-preamp",
                    eqz_preset_10b[preset]->f_preamp );

    setValues( psz_values, eqz_preset_10b[preset]->f_preamp );
}

void Equalizer::delCallbacks( aout_instance_t *p_aout )
{
//    var_DelCallback( p_aout, "equalizer-bands", EqzCallback, this );
//    var_DelCallback( p_aout, "equalizer-preamp", EqzCallback, this );
}

void Equalizer::addCallbacks( aout_instance_t *p_aout )
{
//    var_AddCallback( p_aout, "equalizer-bands", EqzCallback, this );
//    var_AddCallback( p_aout, "equalizer-preamp", EqzCallback, this );
}


/**********************************************************************
 * Video filters / Adjust
 **********************************************************************/

/**********************************************************************
 * Audio filters
 **********************************************************************/

/**********************************************************************
 * Extended playbak controls
 **********************************************************************/
