#pragma once

class LevelObjectInputStream;

/*

 code is not used.

class LevelObjectInputStream : ObjectInputStream
{
    private Set<String> autoReplacers = new HashSet<String>();

    public LevelObjectInputStream(InputStream in) throws IOException
    {
        super(in);

        autoReplacers.add("minecraft.player.Player$1");
        autoReplacers.add("minecraft.mob.Creeper$1");
        autoReplacers.add("minecraft.mob.Skeleton$1");
    }

protected:
	ObjectStreamClass readClassDescriptor() // throws IOException, ClassNotFoundException
    {
        ObjectStreamClass osc = super.readClassDescriptor();

        if (autoReplacers.contains(osc.getName()))
        {
            return ObjectStreamClass.lookup(Class.forName(osc.getName()));
        }
        return osc;
    }
}
*/