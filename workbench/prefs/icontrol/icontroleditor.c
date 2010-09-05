/*
   Copyright � 1995-2010, The AROS Development Team. All rights reserved.
   $Id$

Desc:
Lang: English
 */

#include <aros/debug.h>

#include <devices/rawkeycodes.h>
#include <mui/HotkeyString_mcc.h>
#include <zune/prefseditor.h>
#include <zune/customclasses.h>

#include <proto/commodities.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/alib.h>

#include <string.h>
#include <stdio.h>

#include "icontroleditor.h"
#include "prefs.h"
#include "locale.h"

#include "menupopup3d_image.c"
#include "menupopupclassic_image.c"
#include "menupulldown3d_image.c"
#include "menupulldownclassic_image.c"

#if MENUPOPUP3D_PACKED
static UBYTE menupopup3d_imagedata[MENUPOPUP3D_WIDTH * MENUPOPUP3D_HEIGHT];
#else
#define menupopup3d_imagedata menupopup3d_data
#endif

#if MENUPOPUPCLASSIC_PACKED
static UBYTE menupopupclassic_imagedata[MENUPOPUPCLASSIC_WIDTH * MENUPOPUPCLASSIC_HEIGHT];
#else
#define menupopupclassic_imagedata menupopupclassic_data
#endif

#if MENUPULLDOWN3D_PACKED
static UBYTE menupulldown3d_imagedata[MENUPULLDOWN3D_WIDTH * MENUPULLDOWN3D_HEIGHT];
#else
#define menupulldown3d_imagedata menupulldown3d_data
#endif

#if MENUPULLDOWNCLASSIC_PACKED
static UBYTE menupulldownclassic_imagedata[MENUPULLDOWNCLASSIC_WIDTH * MENUPULLDOWNCLASSIC_HEIGHT];
#else
#define menupulldownclassic_imagedata menupulldownclassic_data
#endif

/*********************************************************************************************/

struct IControlEditor_DATA
{
    Object  *menutypeobj;
    Object  *menulookobj;
    Object  *menustickobj;
    Object  *offscreenobj;
    Object  *defpubscrobj;
    Object  *scrleftdragobj;
    Object  *scrrightdragobj;
    Object  *scrtopdragobj;
    Object  *scrbotdragobj;
    Object  *metadragobj;
};

/*********************************************************************************************/

#define SETUP_INST_DATA struct IControlEditor_DATA *data = INST_DATA(CLASS, self)

/*********************************************************************************************/

static struct Hook  previewhook;
static STRPTR       menutype_labels[3], menulook_labels[3];
static WORD         imagetransparray[4];

/*********************************************************************************************/

static void InitImagePal(ULONG *pal, WORD numcols, WORD index)
{
    WORD i;

    imagetransparray[index] = -1;
    for(i = 0; i < numcols; i++)
    {
        if ((pal[i] & 0xFCFCFC) == 0xFC00FC)
        {
            imagetransparray[index] = i;
        }

        ((UBYTE *)pal)[i * 3 + 0] = pal[i] >> 16;
        ((UBYTE *)pal)[i * 3 + 1] = pal[i] >> 8;
        ((UBYTE *)pal)[i * 3 + 2] = pal[i];
    }
}

/*********************************************************************************************/

#if MENUPOPUP3D_PACKED || MENUPOPUPCLASSIC_PACKED ||MENUPULLDOWN3D_PACKED || MENUPULLDOWNCLASSIC_PACKED

static UBYTE *unpack_byterun1(UBYTE *source, UBYTE *dest, LONG unpackedsize)
{
    UBYTE r;
    BYTE c;

    for(;;)
    {
        c = (BYTE)(*source++);
        if (c >= 0)
        {
            while(c-- >= 0)
            {
                *dest++ = *source++;
                if (--unpackedsize <= 0) return source;
            }
        }
        else if (c != -128)
        {
            c = -c;
            r = *source++;

            while(c-- >= 0)
            {
                *dest++ = r;
                if (--unpackedsize <= 0) return source;
            }
        }
    }
}

#endif

/*********************************************************************************************/

static void InitImages(void)
{
#if MENUPOPUP3D_PACKED
    unpack_byterun1(menupopup3d_data, menupopup3d_imagedata, sizeof(menupopup3d_imagedata));
#endif

#if MENUPOPUPCLASSIC_PACKED
    unpack_byterun1(menupopupclassic_data, menupopupclassic_imagedata, sizeof(menupopupclassic_imagedata));
#endif

#if MENUPULLDOWN3D_PACKED
    unpack_byterun1(menupulldown3d_data, menupulldown3d_imagedata, sizeof(menupulldown3d_imagedata));
#endif

#if MENUPULLDOWNCLASSIC_PACKED
    unpack_byterun1(menupulldownclassic_data, menupulldownclassic_imagedata, sizeof(menupulldownclassic_imagedata));
#endif

    InitImagePal(menupulldownclassic_pal, MENUPULLDOWNCLASSIC_COLORS, 0);
    InitImagePal(menupulldown3d_pal, MENUPULLDOWN3D_COLORS, 1);
    InitImagePal(menupopupclassic_pal, MENUPOPUPCLASSIC_COLORS, 2);
    InitImagePal(menupopup3d_pal, MENUPOPUP3D_COLORS, 3);

}

/*********************************************************************************************/

static void PreviewFunc(struct Hook *hook, Object *previewpage, struct IControlEditor_DATA **data)
{
    IPTR type = 0;
    IPTR look = 0;

    get((*data)->menutypeobj, MUIA_Cycle_Active, &type);
    get((*data)->menulookobj, MUIA_Cycle_Active, &look);

    nnset(previewpage, MUIA_Group_ActivePage, type * 2 + look);
}

/*********************************************************************************************/

BOOL Gadgets2IControlPrefs(struct IControlEditor_DATA *data)
{
    struct IControlPrefs *prefs = &icontrolprefs;

    IPTR active = 0;
    STRPTR key = NULL;
    struct InputXpression ix = {IX_VERSION, 0};

    get(data->menutypeobj, MUIA_Cycle_Active, &active);
    if (active == 0)
    {
        prefs->ic_Flags &= ~ICF_POPUPMENUS;
    }
    else
    {
        prefs->ic_Flags |= ICF_POPUPMENUS;
    }

    get(data->menulookobj, MUIA_Cycle_Active, &active);
    if (active == 0)
    {
        prefs->ic_Flags &= ~ICF_3DMENUS;
    }
    else
    {
        prefs->ic_Flags |= ICF_3DMENUS;
    }

    get(data->menustickobj, MUIA_Selected, &active);
    if (active == 0)
    {
        prefs->ic_Flags &= ~ICF_STICKYMENUS;
    }
    else
    {
        prefs->ic_Flags |= ICF_STICKYMENUS;
    }

    get(data->offscreenobj, MUIA_Selected, &active);
    if (active == 0)
    {
        prefs->ic_Flags &= ~ICF_OFFSCREENLAYERS;
    }
    else
    {
        prefs->ic_Flags |= ICF_OFFSCREENLAYERS;
    }

    get(data->defpubscrobj, MUIA_Selected, &active);
    if (active == 0)
    {
        prefs->ic_Flags &= ~ICF_DEFPUBSCREEN;
    }
    else
    {
        prefs->ic_Flags |= ICF_DEFPUBSCREEN;
    }

    prefs->ic_VDragModes[0] = 0;
    get(data->scrleftdragobj, MUIA_Selected, &active);
    if (active)
        prefs->ic_VDragModes[0] |= ICVDM_LBOUND;
    get(data->scrrightdragobj, MUIA_Selected, &active);
    if (active)
        prefs->ic_VDragModes[0] |= ICVDM_RBOUND;
    get(data->scrtopdragobj, MUIA_Selected, &active);
    if (active)
        prefs->ic_VDragModes[0] |= ICVDM_TBOUND;
    get(data->scrbotdragobj, MUIA_Selected, &active);
    if (active)
        prefs->ic_VDragModes[0] |= ICVDM_BBOUND;

    get(data->metadragobj, MUIA_String_Contents, (IPTR *)&key);
    if (!ParseIX(key, &ix))
        prefs->ic_MetaDrag = ix.ix_Qualifier;

    prefs->ic_Flags |= ICF_AVOIDWINBORDERERASE;

    return TRUE;
}

/*********************************************************************************************/

BOOL IControlPrefs2Gadgets(struct IControlEditor_DATA *data)
{
    struct IControlPrefs *prefs = &icontrolprefs;

    struct InputXpression ix =
    {
        IX_VERSION,
        IECLASS_RAWKEY,
        RAWKEY_LAMIGA,
        0,
        0,
        IX_NORMALQUALS,
        0
    };

    set(data->menutypeobj, MUIA_Cycle_Active, (prefs->ic_Flags & ICF_POPUPMENUS) ? 1 : 0);
    set(data->menulookobj, MUIA_Cycle_Active, (prefs->ic_Flags & ICF_3DMENUS) ? 1 : 0);
    set(data->menustickobj, MUIA_Selected, (prefs->ic_Flags & ICF_STICKYMENUS) ? 1 : 0);
    set(data->offscreenobj, MUIA_Selected, (prefs->ic_Flags & ICF_OFFSCREENLAYERS) ? 1 : 0);
    set(data->defpubscrobj, MUIA_Selected, (prefs->ic_Flags & ICF_DEFPUBSCREEN) ? 1 : 0);
    set(data->scrleftdragobj, MUIA_Selected, prefs->ic_VDragModes[0] & ICVDM_LBOUND);
    set(data->scrrightdragobj, MUIA_Selected, prefs->ic_VDragModes[0] & ICVDM_RBOUND);
    set(data->scrtopdragobj, MUIA_Selected, prefs->ic_VDragModes[0] & ICVDM_TBOUND);
    set(data->scrbotdragobj, MUIA_Selected, prefs->ic_VDragModes[0] & ICVDM_BBOUND);

    ix.ix_Qualifier = prefs->ic_MetaDrag;
    set(data->metadragobj, MUIA_HotkeyString_IX, &ix);

    return TRUE;
}

/*********************************************************************************************/

IPTR IControlEditor__OM_NEW
(
    Class *CLASS, Object *self, struct opSet *message
)
{
    struct IControlEditor_DATA *data = NULL;
    Object *previewpage, *menutypeobj, *menulookobj;
    Object *menustickobj;
    Object *offscreenobj;
    Object *defpubscrobj;
    Object *scrleftdragobj, *scrrightdragobj, *scrtopdragobj, *scrbotdragobj;
    Object *metadragobj;

    InitImages();

    previewhook.h_Entry = HookEntry;
    previewhook.h_SubEntry = (HOOKFUNC)PreviewFunc;

    /*
        WARNING: All Prefs structs must be initialized at this point!
     */

    menutype_labels[0] = _(MSG_MENUS_TYPE_PULLDOWN);
    menutype_labels[1] = _(MSG_MENUS_TYPE_POPUP);

    menulook_labels[0] = _(MSG_MENUS_LOOK_CLASSIC);
    menulook_labels[1] = _(MSG_MENUS_LOOK_3D);

    D(printf("Creating window object...\n"));

    self = (Object *) DoSuperNewTags
    (
        CLASS, self, NULL,

        MUIA_PrefsEditor_Name,     __(MSG_WINTITLE),
        MUIA_PrefsEditor_Path,     (IPTR) "SYS/icontrol.prefs",
        MUIA_PrefsEditor_IconTool, (IPTR) "SYS:Prefs/IControl",

        Child, HGroup,
            Child, (IPTR)VGroup,
                MUIA_Weight, 0,
                GroupFrameT(_(MSG_MENUS_GROUP)),
                Child, ColGroup(2),
                    Child, Label1(_(MSG_MENUS_TYPE)),
                    Child, menutypeobj = MUI_MakeObject(MUIO_Cycle, NULL, menutype_labels),
                    Child, Label1(_(MSG_MENUS_LOOK)),
                    Child, menulookobj = MUI_MakeObject(MUIO_Cycle, NULL, menulook_labels),
                End,
                Child, ColGroup(3),
                    Child, HSpace(0),
                    Child, Label1(_(MSG_MENUS_STICKY)),
                    Child, menustickobj = MUI_MakeObject(MUIO_Checkmark, NULL),
                End,
                Child, VSpace(1),
                Child, previewpage = PageGroup,
                    //ImageButtonFrame,
                    MUIA_Background, (IPTR)"2:6c6c6c6c,6a6a6a6a,b5b5b5b5",
                    Child, ChunkyImageObject,
                        MUIA_ChunkyImage_Pixels, (IPTR)menupulldownclassic_imagedata,
                        MUIA_ChunkyImage_Palette, (IPTR)menupulldownclassic_pal,
                        MUIA_ChunkyImage_NumColors, MENUPULLDOWNCLASSIC_COLORS,
                        MUIA_Bitmap_Width, MENUPULLDOWNCLASSIC_WIDTH,
                        MUIA_Bitmap_Height, MENUPULLDOWNCLASSIC_HEIGHT,
                        MUIA_FixWidth, MENUPULLDOWNCLASSIC_WIDTH,
                        MUIA_FixHeight, MENUPULLDOWNCLASSIC_HEIGHT,
                        MUIA_Bitmap_UseFriend, TRUE,
                        MUIA_Bitmap_Transparent, imagetransparray[0],
                    End,
                    Child, ChunkyImageObject,
                        MUIA_ChunkyImage_Pixels, (IPTR)menupulldown3d_imagedata,
                        MUIA_ChunkyImage_Palette, (IPTR)menupulldown3d_pal,
                        MUIA_ChunkyImage_NumColors, MENUPULLDOWN3D_COLORS,
                        MUIA_Bitmap_Width, MENUPULLDOWN3D_WIDTH,
                        MUIA_Bitmap_Height, MENUPULLDOWN3D_HEIGHT,
                        MUIA_FixWidth, MENUPULLDOWN3D_WIDTH,
                        MUIA_FixHeight, MENUPULLDOWN3D_HEIGHT,
                        MUIA_Bitmap_UseFriend, TRUE,
                        MUIA_Bitmap_Transparent, imagetransparray[1],
                    End,
                    Child, ChunkyImageObject,
                        MUIA_ChunkyImage_Pixels, (IPTR)menupopupclassic_imagedata,
                        MUIA_ChunkyImage_Palette, (IPTR)menupopupclassic_pal,
                        MUIA_ChunkyImage_NumColors, MENUPOPUPCLASSIC_COLORS,
                        MUIA_Bitmap_Width, MENUPOPUPCLASSIC_WIDTH,
                        MUIA_Bitmap_Height, MENUPOPUPCLASSIC_HEIGHT,
                        MUIA_FixWidth, MENUPOPUPCLASSIC_WIDTH,
                        MUIA_FixHeight, MENUPOPUPCLASSIC_HEIGHT,
                        MUIA_Bitmap_UseFriend, TRUE,
                        MUIA_Bitmap_Transparent, imagetransparray[2],
                    End,
                    Child, ChunkyImageObject,
                        MUIA_ChunkyImage_Pixels, (IPTR)menupopup3d_imagedata,
                        MUIA_ChunkyImage_Palette, (IPTR)menupopup3d_pal,
                        MUIA_ChunkyImage_NumColors, MENUPOPUP3D_COLORS,
                        MUIA_Bitmap_Width, MENUPOPUP3D_WIDTH,
                        MUIA_Bitmap_Height, MENUPOPUP3D_HEIGHT,
                        MUIA_FixWidth, MENUPOPUP3D_WIDTH,
                        MUIA_FixHeight, MENUPOPUP3D_HEIGHT,
                        MUIA_Bitmap_UseFriend, TRUE,
                        MUIA_Bitmap_Transparent, imagetransparray[3],
                    End,
                End, // PageGroup
            End,
            Child, VGroup,
                Child, VGroup,
                    GroupFrameT(_(MSG_WINDOWS)),
                    Child, VSpace(0),
                    Child, ColGroup(4),
                        Child, HSpace(0),
                        Child, Label1(_(MSG_OFFSCREEN_MOVE)),
                        Child, offscreenobj = MUI_MakeObject(MUIO_Checkmark, NULL),
                        Child, HSpace(0),
                    End,
                    Child, VSpace(0),
                End,
                Child, VGroup,
                    GroupFrameT(_(MSG_SCREENS)),
                    Child, VSpace(0),
                    Child, ColGroup(2),
                        Child, Label1(_(MSG_FRONTMOST_DEFAULT)),
                        Child, HGroup,
                            Child, defpubscrobj = MUI_MakeObject(MUIO_Checkmark, NULL),
                            Child, HSpace(0),
                        End,
                        Child, Label1(_(MSG_BOUND_LEFT_DRAG)),
                        Child, HGroup,
                            Child, scrleftdragobj = MUI_MakeObject(MUIO_Checkmark, NULL),
                            Child, HSpace(0),
                        End,
                        Child, Label1(_(MSG_BOUND_RIGHT_DRAG)),
                        Child, HGroup,
                            Child, scrrightdragobj = MUI_MakeObject(MUIO_Checkmark, NULL),
                            Child, HSpace(0),
                        End,
                        Child, Label1(_(MSG_BOUND_TOP_DRAG)),
                        Child, HGroup,
                            Child, scrtopdragobj = MUI_MakeObject(MUIO_Checkmark, NULL),
                            Child, HSpace(0),
                        End,
                        Child, Label1(_(MSG_BOUND_BOTTOM_DRAG)),
                        Child, HGroup,
                            Child, scrbotdragobj = MUI_MakeObject(MUIO_Checkmark, NULL),
                            Child, HSpace(0),
                        End,
                        Child, Label1(_(MSG_META_DRAG)),
                        Child, metadragobj = MUI_NewObject("HotKeyString.mcc",
                            MUIA_Frame, MUIV_Frame_String,
                        TAG_DONE),
                    End,
                    Child, VSpace(0),
                End,
            End,
        End,
        TAG_DONE
    );

    D(printf("Created prefs window object 0x%p\n", self));

    if (self == NULL) goto error;

    data = INST_DATA(CLASS, self);
    data->menutypeobj = menutypeobj;
    data->menulookobj = menulookobj;
    data->menustickobj = menustickobj;
    data->offscreenobj = offscreenobj;
    set(data->offscreenobj, MUIA_ShortHelp, __(MSG_OFFSCREEN_DESC));
    data->defpubscrobj = defpubscrobj;
    data->scrleftdragobj = scrleftdragobj;
    data->scrrightdragobj = scrrightdragobj;
    data->scrtopdragobj = scrtopdragobj;
    data->scrbotdragobj = scrbotdragobj;
    data->metadragobj = metadragobj;
    set(data->defpubscrobj, MUIA_ShortHelp, __(MSG_FRONTMOST_DEFAULT_DESC));

    DoMethod
    (
        menutypeobj, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
        (IPTR)previewpage, 3, MUIM_CallHook, (IPTR)&previewhook, (IPTR)data
    );
    DoMethod
    (
        menulookobj, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
        (IPTR)previewpage, 3, MUIM_CallHook, (IPTR)&previewhook, (IPTR)data
    );

    IControlPrefs2Gadgets(data);

    return (IPTR) self;

error:

    return 0;
}

/*********************************************************************************************/

IPTR IControlEditor__MUIM_PrefsEditor_ImportFH (
    Class *CLASS, Object *self,
    struct MUIP_PrefsEditor_ImportFH *message
)
{
    SETUP_INST_DATA;
    BOOL success = TRUE;

    D(bug("[icontroleditor class] IControlEditor Class Import\n"));

    success = Prefs_ImportFH(message->fh);
    if (success) IControlPrefs2Gadgets(data);

    return success;
}

/*********************************************************************************************/

IPTR IControlEditor__MUIM_PrefsEditor_ExportFH
(
    Class *CLASS, Object *self,
    struct MUIP_PrefsEditor_ExportFH *message
)
{
    SETUP_INST_DATA;
    BOOL success = TRUE;

    D(bug("[icontroleditor class] IControlEditor Class Export\n"));

    Gadgets2IControlPrefs(data);
    success = Prefs_ExportFH(message->fh);

    return success;
}

/*********************************************************************************************/

IPTR IControlEditor__MUIM_PrefsEditor_SetDefaults
(
    Class *CLASS, Object *self, Msg message
)
{
    SETUP_INST_DATA;
    BOOL success = TRUE;

    D(bug("[icontroleditor class] IControlEditor Class SetDefaults\n"));

    success = Prefs_Default();
    if (success) IControlPrefs2Gadgets(data);

    return success;
}

/*** Setup ******************************************************************/
ZUNE_CUSTOMCLASS_4
(
    IControlEditor, NULL, MUIC_PrefsEditor, NULL,
    OM_NEW,                       struct opSet *,
    MUIM_PrefsEditor_ImportFH,    struct MUIP_PrefsEditor_ImportFH *,
    MUIM_PrefsEditor_ExportFH,    struct MUIP_PrefsEditor_ExportFH *,
    MUIM_PrefsEditor_SetDefaults, Msg
);
