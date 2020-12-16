package com.nexstreaming.app.common.nexasset.assetpackage.security;

import java.io.IOException;
import java.io.InputStream;

public interface PackageSecurityManager {
    InputStream wrapInputStream(InputStream inputStream, String path) throws IOException;
    boolean plaintextAvailable(String path);
}
