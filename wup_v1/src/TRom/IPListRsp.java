// **********************************************************************
// This file was generated by a TAF parser!
// TAF version 3.2.1.1 by WSRD Tencent.
// Generated from `/usr/local/resin_system.mqq.com/webapps/communication/taf/upload/sukeyli/LoginQ.jce'
// **********************************************************************

package TRom;

public final class IPListRsp extends com.qq.taf.jce.JceStruct implements java.lang.Cloneable
{
    public String className()
    {
        return "TRom.IPListRsp";
    }

    public String fullClassName()
    {
        return "TRom.IPListRsp";
    }

    public java.util.ArrayList<TRom.JoinIPInfo> vJoinIPInfo = null;

    public String sClientIp = "";

    public java.util.ArrayList<TRom.JoinIPInfo> getVJoinIPInfo()
    {
        return vJoinIPInfo;
    }

    public void  setVJoinIPInfo(java.util.ArrayList<TRom.JoinIPInfo> vJoinIPInfo)
    {
        this.vJoinIPInfo = vJoinIPInfo;
    }

    public String getSClientIp()
    {
        return sClientIp;
    }

    public void  setSClientIp(String sClientIp)
    {
        this.sClientIp = sClientIp;
    }

    public IPListRsp()
    {
    }

    public IPListRsp(java.util.ArrayList<TRom.JoinIPInfo> vJoinIPInfo, String sClientIp)
    {
        this.vJoinIPInfo = vJoinIPInfo;
        this.sClientIp = sClientIp;
    }

    public boolean equals(Object o)
    {
        if(o == null)
        {
            return false;
        }

        IPListRsp t = (IPListRsp) o;
        return (
            com.qq.taf.jce.JceUtil.equals(vJoinIPInfo, t.vJoinIPInfo) && 
            com.qq.taf.jce.JceUtil.equals(sClientIp, t.sClientIp) );
    }

    public int hashCode()
    {
        try
        {
            throw new Exception("Need define key first!");
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
        }
        return 0;
    }
    public java.lang.Object clone()
    {
        java.lang.Object o = null;
        try
        {
            o = super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert false; // impossible
        }
        return o;
    }

    public void writeTo(com.qq.taf.jce.JceOutputStream _os)
    {
        if (null != vJoinIPInfo)
        {
            _os.write(vJoinIPInfo, 0);
        }
        if (null != sClientIp)
        {
            _os.write(sClientIp, 1);
        }
    }

    static java.util.ArrayList<TRom.JoinIPInfo> cache_vJoinIPInfo;

    public void readFrom(com.qq.taf.jce.JceInputStream _is)
    {
        if(null == cache_vJoinIPInfo)
        {
            cache_vJoinIPInfo = new java.util.ArrayList<TRom.JoinIPInfo>();
            TRom.JoinIPInfo __var_5 = new TRom.JoinIPInfo();
            ((java.util.ArrayList<TRom.JoinIPInfo>)cache_vJoinIPInfo).add(__var_5);
        }
        this.vJoinIPInfo = (java.util.ArrayList<TRom.JoinIPInfo>) _is.read(cache_vJoinIPInfo, 0, false);
        this.sClientIp =  _is.readString(1, false);
    }

    public void display(java.lang.StringBuilder _os, int _level)
    {
        com.qq.taf.jce.JceDisplayer _ds = new com.qq.taf.jce.JceDisplayer(_os, _level);
        _ds.display(vJoinIPInfo, "vJoinIPInfo");
        _ds.display(sClientIp, "sClientIp");
    }

    public void displaySimple(java.lang.StringBuilder _os, int _level)
    {
        com.qq.taf.jce.JceDisplayer _ds = new com.qq.taf.jce.JceDisplayer(_os, _level);
        _ds.displaySimple(vJoinIPInfo, true);
        _ds.displaySimple(sClientIp, false);
    }

}

