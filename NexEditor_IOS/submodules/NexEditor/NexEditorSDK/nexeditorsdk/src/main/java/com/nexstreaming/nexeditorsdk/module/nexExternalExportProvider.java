package com.nexstreaming.nexeditorsdk.module;

import com.nexstreaming.nexeditorsdk.nexExportFormat;

/**
 *
 * @since 2.0.0
 */
public interface nexExternalExportProvider extends nexModuleProvider, nexExternalExportMethod {
    /**
     *
     * @since 2.0.0
     */
    public final static String[] nexExportFormatMandatoryKeys = {
            nexExportFormat.TAG_FORMAT_TYPE,
            nexExportFormat.TAG_FORMAT_PATH,
            nexExportFormat.TAG_FORMAT_WIDTH,
            nexExportFormat.TAG_FORMAT_HEIGHT,
            nexExportFormat.TAG_FORMAT_INTERVAL_TIME,
            nexExportFormat.TAG_FORMAT_START_TIME,
            nexExportFormat.TAG_FORMAT_END_TIME
    };

    /**
     *
     * @since 2.0.0
     */
    public final static String[] nexExportFormatOptionalKeys = {
            nexExportFormat.TAG_FORMAT_UUID,
            nexExportFormat.TAG_FORMAT_QUALITY,
    };

}
