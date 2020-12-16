package com.nexstreaming.app.common.nexasset.assetpackage;

import com.nexstreaming.app.common.task.ResultTask;

import java.io.File;
import java.util.List;

public interface HTTPDownloader {

    // TODO!!
    ResultTask<File> download(String url);
    ResultTask<List<File>> downloadSeveral(String... url);

}
