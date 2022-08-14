package android.net.ethernet;
import android.net.ethernet.EthernetDevInfo;

/**
 * Interface that allows controlling and querying Ethernet connectivity.
 *
 * {@hide}
 */
interface IEthernetManager
{
	String[] getDeviceNameList();
	void setEthState(int state);
	int getEthState( );
	void UpdateEthDevInfo(in EthernetDevInfo info);
	boolean isEthConfigured();
	EthernetDevInfo getSavedEthConfig();
	int getTotalInterface();
	void setEthMode(String mode);
	int getCheckConnecting();
	void setCheckConnecting(int value);
	boolean getStackConnected();
	void setStackConnected(boolean value);
	boolean getHWConnected();
	void setHWConnected(boolean value);
	boolean isEthDeviceFound();
}
