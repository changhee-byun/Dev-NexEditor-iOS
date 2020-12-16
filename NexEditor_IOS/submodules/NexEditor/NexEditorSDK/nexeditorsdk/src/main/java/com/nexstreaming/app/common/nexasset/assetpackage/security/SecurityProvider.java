package com.nexstreaming.app.common.nexasset.assetpackage.security;

public interface SecurityProvider {
    String getProviderId();
    String[] getEncInfo();
    PackageSecurityManager getSecurityManagerForPackage(String providerSpecificData);
}
