/******************************************************************************
 NexTheme.h -- Header for theme manager
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2011 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Description:
 
    This file includes functions for managing ThemeSet, Theme and Effect
    objects.
 
    A ThemeSet is a group of one or more themes, which can be loaded from a
    file or from a string in memory.  Themes are defined using a theme language
    that is similar to XML.  The general structure is as follows:
 
        <theme id="...">
            <effect id="...">
            ... effect definition ...
            </effect>
              .
              .
              .
            <effect id="...">
            ... effect definition ...
            </effect>
        </theme>
          .
          .
          .
        <theme id="...">
            <effect id="...">
            ... effect definition ...
            </effect>
        </theme>
 
    ThemeSet object are tracked by reference count, to prevent a ThemeSet being
    deallocated while one of it's themes is still attached to a renderer.  When
    creating a theme set, the caller owns the returned reference, and should
    release it when done.
 
    All objects inside a ThemeSet are created and destroyed at the same time
    with the ThemeSet they belong to.
 
    Themes and effects are identified by the "id" tag in the theme language.
    The ThemeSet API provides easy access to the ID.
 
    To list the IDs of all themes in a ThemeSet:
 
        NXT_HThemeSet hThemeSet = NXT_ThemeSet_CreateFromString( themedata );
        unsigned int numberOfThemes = NXT_ThemeSet_GetThemeCount( hThemeSet );
        for( unsigned int i=0; i<numberOfThemes; i++ ) {
            NXT_HTheme hTheme = NXT_ThemeSet_GetTheme( hThemeSet, i );
            char *theme_id = NXT_Theme_GetID( hTheme );
            printf( "%u: %s\n", i, theme_id );
        }
        NXT_ThemeSet_Release( hThemeSet );
        hThemeSet = NULL;
 
    To get a specific theme if you know the ID:
 
        NXT_HTheme themeByID( hThemeSet, char *theme_id ) {
            unsigned int numberOfThemes = NXT_ThemeSet_GetThemeCount( hThemeSet );
            for( unsigned int i=0; i<numberOfThemes; i++ ) {
                NXT_HTheme hTheme = NXT_ThemeSet_GetTheme( hThemeSet, i );
                if( strcmp(NXT_Theme_GetID( hTheme ),theme_id)==0 ) {
                    return hTheme;
                }
            }
            return NULL;
        }
 
    Effects work in the same way.  To use a theme, you have to attach one of the
    effects to a renderer.  See NexThemeRenderer.h for details.
 
 ******************************************************************************/

#ifdef NXT_ALLOW_DIRECT_THEME_ACCESS
#ifndef NEXTHEME_H
#define NEXTHEME_H

#define NEXTHEME_VERSION_MAJOR  0
#define NEXTHEME_VERSION_MINOR  1
#define NEXTHEME_VERSION_PATCH  3
#define NEXTHEME_VERSION        "0.1.3"

    // --- Opaque Handles ----------------------------------------------------------
    struct NXT_ThemeSet_;
    struct NXT_Theme_;
    struct NXT_Effect_;
    struct NXT_ThemeRenderer_;
    struct NXT_ThemeRenderer_Context;
    typedef struct NXT_ThemeRenderer_Context *NXT_HThemeRenderer_Context;
    typedef struct NXT_ThemeRenderer_ *NXT_HThemeRenderer;
    typedef struct NXT_ThemeSet_    NXT_ThemeSet;
    typedef struct NXT_ThemeSet_    *NXT_HThemeSet;
    typedef struct NXT_Theme_       NXT_Theme;
    typedef struct NXT_Theme_       *NXT_HTheme;
    typedef struct NXT_Effect_      NXT_Effect;
    typedef struct NXT_Effect_      *NXT_HEffect;
    
    // --- Types -------------------------------------------------------------------
    
    typedef enum NXT_PartType_ {
        NXT_PartType_All = 0,
        NXT_PartType_In = 1,
        NXT_PartType_Out = 2,
        NXT_PartType_Mid = 3
    } NXT_PartType;
    
    typedef enum NXT_EffectType_ {
        NXT_EffectType_NONE         = 0,
        NXT_EffectType_Transition   = 1,
        NXT_EffectType_Title        = 2
    } NXT_EffectType;
    
    typedef enum NXT_EffectProperty_ {
        
        /**
         * The effect type.
         *
         * This is the same value as returned by NXT_Effect_GetType(), and is
         * either NXT_EffectType_Transition or NXT_EffectType_Title
         */
        NXT_EffectProperty_EffectType           = 1,
        
        /**
         * The offset of the effect from the end of the first clip.
         *
         * This applies only to transitions.  This is a value between 0 and 100
         * and indicates a proportion of the duration of the effect.
         *
         * For example:
         *
         *                     |<------| (50% offset)
         *                     |--Transition---|
         *     |-----Video 1-----------|
         *                             |-----Video 2-----------|
         *
         *
         *
         *                     |<--------------| (100% offset)
         *                     |--Transition---|
         *     |-----Video 1-------------------|
         *                     |-----Video 2-----------|
         *
         */
        NXT_EffectProperty_EffectOffset         = 2,
        
        /**
         * The amount by which two video clips overlap when this effect is used
         * to transition between them.
         *
         * This applies only to transitions.  The value is between 0 and 100 and
         * indicates a percentage of the duratiion of the effect.
         *
         * For example:
         *
         *                     |--Transition---|
         *     |-----Video 1-----------|
         *                             |-----Video 2-----------|
         *
         *                             |
         *                         (0% overlap)
         *
         *
         *                     |--Transition---|
         *     |-----Video 1---------------|
         *                         |-----Video 2-----------|
         *
         *                         \_______/
         *                         (50% overlap)
         *
         *
         *                     |--Transition---|
         *     |-----Video 1-------------------|
         *                     |-----Video 2-----------|
         *
         *                     \_______________/
         *                      (100% overlap)
         *
         */
        NXT_EffectProperty_VideoOverlap         = 3,
        
        /**
         * The number of title text strings accepted
         *
         * This applies only to title effects (NXT_EffectType_Title) and
         * indicates how many text strings can be displayed by the title effect.
         * Usually, this is 1, but for title effects that display text in
         * multiple locations, this could be a larger number.
         */
        NXT_EffectProperty_NumTitleStrings      = 4,
        
        /**
         * Indicates if this is a global effect.
         *
         * 1:  This effect is global (defined at the ThemeSet level)
         * 0:  This effect is not global (defined at the Theme level)
         */
        NXT_EffectProperty_IsGlobal             = 5,
        
        /**
         * Indicates the minimumn duration allowed for this transition
         */
        NXT_EffectProperty_MinimumDuration      = 6,
        
        /**
         * Indicates the maximum duration allowed for this transition
         */
        NXT_EffectProperty_MaximumDuration      = 7,
        
        /**
         * Indicates the default duration this transition
         */
        NXT_EffectProperty_DefaultDuration      = 8
        
        
    } NXT_EffectProperty;
    
    typedef enum NXT_ThemeSetProperty_ {
        NXT_ThemeSetProperty_DefaultTheme               = 1,
        NXT_ThemeSetProperty_DefaultTransitionEffect    = 2,
        NXT_ThemeSetProperty_DefaultTitleEffect         = 3
    } NXT_ThemeSetProperty;
    
    
    // --- General Functions ------------------------------------------------------
    
    int NXT_ThemeManager_GetVersionInt( int iComponent );
    
    // --- ThemeSet Functions ------------------------------------------------------
    
#ifdef _STDIO_H_
    /**
     * Creates a ThemeSet object by reading and parsing the given file.
     *
     * When you are done with the ThemeSet, release it by calling
     * NXT_ThemeSet_Release().  The ThemeSet is created with an initial
     * reference count of 1.
     */
    NXT_HThemeSet   NXT_ThemeSet_CreateFromFile( FILE* file );
#endif //_STDIO_H_
    
    /**
     * Creates a ThemeSet object by parsing data in the given string.
     *
     * When you are done with the ThemeSet, release it by calling
     * NXT_ThemeSet_Release().  The ThemeSet is created with an initial
     * reference count of 1.
     */
    NXT_HThemeSet   NXT_ThemeSet_CreateFromString( const char* string );
    
    /**
     * Increments the reference count of the ThemeSet.
     */
    NXT_HThemeSet   NXT_ThemeSet_Retain( NXT_HThemeSet themeSet );
    
    /**
     * Returns a string property relating to the theme set.
     */
    char*           NXT_ThemeSet_GetStringProperty( NXT_HThemeSet themeSet, NXT_ThemeSetProperty property );
    
    /**
     * Decrements the reference count of the ThemeSet.  If it reaches zero,
     * the ThemeSet is destroyed, and all Theme and Effect objects inside of
     * it are destroyed.
     */
    int            NXT_ThemeSet_Release(NXT_HThemeRenderer_Context context, NXT_HThemeSet themeSet, unsigned int isDetachedContext );
    
    /**
     * Returns the number of themes in a ThemeSet.
     */
    unsigned int    NXT_ThemeSet_GetThemeCount( NXT_HThemeSet themeSet );
    
    /**
     * Returns a handle to one of the themes in a ThemeSet.
     *
     * \param index     A zero-based index of the theme.  To get the total
     *                  number of themes, call ::NXT_ThemeSet_GetThemeCount.
     */
    NXT_HTheme      NXT_ThemeSet_GetTheme( NXT_HThemeSet themeSet, unsigned int index );
	
	
	NXT_HEffect		NXT_ThemeSet_GetEffectById( NXT_HThemeSet themeSet, const char *effectId );
    
    // --- Theme Functions ---------------------------------------------------------
    /**
     * Returns the ID of the given theme.
     *
     * The ID is an internal value from the ThemeSet file that is used to
     * unqiuely identify the theme.  It may be any arbitrary value; it should
     * not be displayed to the user (use the theme Name instead for the user
     * interface).
     *
     * The returned value should not be freed, and is valid until the next call
     * to a theme-related API function.
     */
    char*           NXT_Theme_GetID( NXT_HTheme theme );
    
    /**
     * Returns the name of the given theme.
     *
     * The name is a user-friendly description suitable for the in the UI.
     *
     * The returned value should not be freed, and is valid until the next call
     * to a theme-related API function.
     */
    char*           NXT_Theme_GetName( NXT_HTheme theme );
    
    /**
     * Returns the number of effects in a Theme.
     */
    unsigned int    NXT_Theme_GetEffectCount( NXT_HTheme theme );
    
    /**
     * Returns a handle to one of the effects in a Theme.
     *
     * \param index     A zero-based index of the effect.  To get the total
     *                  number of effects, call ::NXT_Theme_GetEffectCount.
     */
    NXT_HEffect     NXT_Theme_GetEffect( NXT_HTheme theme, unsigned int index );

    // --- Effect Functions ---------------------------------------------------------
    /**
     * Returns the ID of the given effect.
     *
     * The ID is an internal value from the ThemeSet file that is used to
     * unqiuely identify the effect.  It may be any arbitrary value; it should
     * not be displayed to the user (use the effect Name instead for the user
     * interface).
     *
     * The returned value should not be freed, and is valid until the next call
     * to a theme-related API function.
     */
    char*           NXT_Effect_GetID( NXT_HEffect effect );
    
    /**
     * Returns the name of the given effect.
     *
     * The name is a user-friendly description suitable for the in the UI.
     *
     * The returned value should not be freed, and is valid until the next call
     * to a theme-related API function.
     */
    char*           NXT_Effect_GetName( NXT_HEffect effect );
    
    /**
     * Returns the name of the icon for the given effect.
     *
     * The returned value should not be freed, and is valid until the next call
     * to a theme-related API function.
     */
    char*           NXT_Effect_GetIcon( NXT_HEffect effect );
    
    /**
     * Returns the type of the given effect.
     *
     * This can be either NXT_EffectType_Transition or NXT_EffectType_Title.
     */
    NXT_EffectType  NXT_Effect_GetType( NXT_HEffect effect );
    
    /**
     * Returns the theme containing the given effect.
     */
    NXT_HTheme NXT_Effect_GetTheme( NXT_HEffect effect );
    
    /**
     * Returns an integer property related to a given effect.
     *
     * \param effect    Handle of the effect about which to get the property.
     * \param property  The property to get.
     * \return          The value of the property (0 if the property argument is invalid)
     */
    int NXT_Effect_GetIntProperty( NXT_HEffect effect, NXT_EffectProperty property );
    
    
    /**
     * Seeks the event to a given position and pre-calculates coordintes of
     * animated elements in preparation for rendering.
     *
     * This should normally not be called directly (the theme rendering will
     * call this automatically, when needed).
     */
    void NXT_Effect_Seek( NXT_HEffect effect, float currentTimeInPart, float currentTimeOverall, NXT_PartType part, int clip_index, int clip_count );
    
    void NXT_Effect_UpdateUserFields( NXT_HEffect effect, void* renderer );

    unsigned int NXT_Theme_GetTotalVideoSlot(NXT_HEffect effect);

    int NXT_Theme_DoPrecache(NXT_HEffect effect, void* renderer, int* asyncmode, int* max_replaceable);

    int NXT_Theme_GetPrecacheResource(NXT_HEffect effect, void* renderer, void* load_resource_list);
#endif //NEXTHEME_H
#endif //NXT_ALLOW_DIRECT_THEME_ACCESS
