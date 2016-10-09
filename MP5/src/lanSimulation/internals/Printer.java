package lanSimulation.internals;

/**
 * Created by JoeSun on 9/30/16.
 */
public class Printer extends Node {
    public Printer(byte _type, java.lang.String _name, Node _nextNode) {
        super(_type, _name, _nextNode);
    }

    public Printer(byte _type, java.lang.String _name) {
        super(_type, _name);
    }

    public StringBuffer stringbuffer(StringBuffer buf, boolean mxl) {
        if (mxl) {
            buf.append("<printer>");
            buf.append(name);
            buf.append("</printer>");
        } else {
            buf.append("printer ");
            buf.append(name);
            buf.append(" [printer]");
        }
        return buf;
    }
}
