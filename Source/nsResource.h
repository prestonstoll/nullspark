// nsResource.h - Defines the interface for a NullSpark resource (base class to nsMesh, etc)
#ifndef NSRESOURCE_H__
#define NSRESOURCE_H__

enum nsEResourceType { NS_RESOURCE_MESH, NS_RESOURCE_TYPE_COUNT };

class nsResource
{
private:
	unsigned int				ID;
protected:
	unsigned int				Hash;
	nsEResourceType				Type;
public:
	// Removes the resource from memory
	virtual int					Destroy( void )
	{
		return 0;
	}

	// getHash
	const unsigned int			getHash( void )
	{
		return Hash;
	}

	// getType
	const nsEResourceType		getType( void )
	{
		return Type;
	}

	// getID
	const unsigned int			getID( void )
	{
		return ID;
	}

	// setID
	void						setID( unsigned int inID )
	{
		ID = inID;
	}
};

#endif