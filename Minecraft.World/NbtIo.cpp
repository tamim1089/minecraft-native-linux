#include "stdafx.h"
#include "InputOutputStream.h"
#include "NbtIo.h"

CompoundTag *NbtIo::readCompressed(InputStream *in)
{
	MemSect(26);
	// this was using a try/finally block
	DataInputStream dis = DataInputStream(in); // was new GZIPInputStream as well
	CompoundTag *ret = NbtIo::read((DataInput *)&dis);
	dis.close();
	MemSect(0);
	return ret;
}

void NbtIo::writeCompressed(CompoundTag *tag, OutputStream *out)
{
	// this was using a try/finally block
	// Buffer output in 1024 byte chunks so that we can allocate properly in the save file
	BufferedOutputStream bos = BufferedOutputStream( out, 1024 );
	DataOutputStream dos = DataOutputStream(&bos); // was new GZIPOutputStream as well
	NbtIo::write(tag, &dos);
	dos.close();
}

// Reads tags from a stream created from the input buffer. Doesn't free the data in the source buffer.
CompoundTag *NbtIo::decompress(byteArray buffer)
{
	ByteArrayInputStream bais = ByteArrayInputStream(buffer);
	// this was using a try/finally block
	DataInputStream in = DataInputStream(&bais); // was new GZIPInputStream as well
	CompoundTag *ret = NbtIo::read((DataInput *)&in);
	bais.reset();		// This stops the buffer referenced by the input stream from being freed when it goes out of context
	in.close();
	return ret;
}

byteArray NbtIo::compress(CompoundTag *tag)
{
	// this was using a try/finally block
    ByteArrayOutputStream baos = ByteArrayOutputStream();
    DataOutputStream dos = DataOutputStream(&baos); // was new GZIPOutputStream as well
	NbtIo::write(tag, &dos);

	byteArray ret(baos.buf.length);
	System::arraycopy(baos.buf,0,&ret,0,baos.buf.length);
	dos.close();
	return ret;
}

CompoundTag *NbtIo::read(DataInput *dis)
{
	Tag *tag = Tag::readNamedTag(dis);

	if( tag->getId() == Tag::TAG_Compound ) return (CompoundTag *)tag;

	if(tag!=NULL) delete tag;
	// Root tag must be a named compound tag
	return NULL;
}

void NbtIo::write(CompoundTag *tag, DataOutput *dos)
{
	Tag::writeNamedTag(tag, dos);
}