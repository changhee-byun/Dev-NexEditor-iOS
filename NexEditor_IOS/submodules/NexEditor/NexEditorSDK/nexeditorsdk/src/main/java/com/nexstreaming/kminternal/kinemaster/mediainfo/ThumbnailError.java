package com.nexstreaming.kminternal.kinemaster.mediainfo;

import android.content.Context;

import com.nexstreaming.app.common.task.Task;

enum ThumbnailError implements Task.TaskError {
    RawFileNotFound, RawFileTooSmall, NoThumbailsFound, UnknownFormat, ParameterError;

    @Override
    public Exception getException() {
        return null;
    }

    @Override
    public String getMessage() {
        return name();
    }

    @Override
    public String getLocalizedMessage(Context context) {
        return name();
    }
}
