package lanSimulation.internals;


/**
 * Created by JoeSun on 9/30/16.
 */
public class WorkStation extends Node{
    public WorkStation(byte _type, java.lang.String _name, Node _nextNode) {
        super(_type, _name, _nextNode);
    }

    public WorkStation(byte _type, java.lang.String _name) {
        super(_type, _name);
    }

    public StringBuffer stringbuffer(StringBuffer buf, boolean mxl) {
        if (mxl) {
            buf.append("<workstation>");
            buf.append(name);
            buf.append("</workstation>");
        } else {
            buf.append("workstation");
            buf.append(name);
            buf.append(" [workstation]");
        }
        return buf;
    }



}
