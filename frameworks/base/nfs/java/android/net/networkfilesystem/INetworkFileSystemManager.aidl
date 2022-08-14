package android.net.networkfilesystem;
import android.net.networkfilesystem.NetworkFileSystemDevInfo;

/**
 * Interface that allows controlling and querying Network file system connectivity.
 *
 * {@hide}
 */
interface INetworkFileSystemManager
{
    void setNfsConfig(in NetworkFileSystemDevInfo info);
    NetworkFileSystemDevInfo getNfsConfig(String label);
    boolean setNfsEnabled(String label, boolean state);
    String getNfsState(String label);
}
