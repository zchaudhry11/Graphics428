using UnityEngine;
using System.Collections;

public class ObstacleMoveTo : MonoBehaviour {
	private NavMeshAgent navAgent;
	//private Animator anim;
	private bool select;
	private bool set = false;
	private Vector3 MoveD;
	// Use this for initialization
	void Start () {
		navAgent = GetComponent<NavMeshAgent>();
	}

	// Update is called once per frame
	void Update () {
		if (select && set)
		{
			navAgent.destination = MoveD;
		}
		if (navAgent.pathStatus == NavMeshPathStatus.PathComplete)
		{
			set = false;
		}
	}
	
	void Select()
	{
		Debug.Log("Cube Logged");
		select = true;
	}
	
	void Deselect(int x)
	{
		Debug.Log("Cube Delogged");
		select = false;
	}

	void Destination (Vector3 dest)
	{
		Debug.Log("Destination Set");
		MoveD = dest;
		set = true;
	}
}
